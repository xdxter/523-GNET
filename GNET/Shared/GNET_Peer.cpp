#include <iostream>
#include <map>
#include "winsock2.h"
#include "GNET_Peer.h"

#define MAX_PACKETSIZE 16

using namespace GNET;

DWORD WINAPI runRecvThread(void* param) { return ((Peer*)param)->recvThread(); }
DWORD WINAPI runSendThread(void* param) { return ((Peer*)param)->sendThread(); }
DWORD WINAPI runLogcThread(void* param) { return ((Peer*)param)->logcThread(); }

Peer::Peer(unsigned int max_packet_size) :
send_buffer(1000),
recv_buffer(1000),
game_recv_buffer(1000)
{	
	PacketEncoder::RegisterNetPackets();
	this->max_packet_size = max_packet_size;
}

Peer::~Peer() 
{
	max_clients = 0;
}

enum {
	WSA_INIT_FAILED,
	SOCK_CREATE_FAILED
};

int Peer::Startup(int max_connections, unsigned short port, int sleep_time)
{
	this->max_connections = max_connections;
	this->sleep_time = sleep_time;

	WSADATA WsaDat;

	int error;
	if ((error = WSAStartup(MAKEWORD(2, 0), &WsaDat)) != 0) {
		WSACleanup();
		return error;
	}

	this->socketID = socket(AF_INET, SOCK_DGRAM, 0);
	if (this->socketID == INVALID_SOCKET) {
		WSACleanup();
		return SOCK_CREATE_FAILED;
	}

	SOCKADDR_IN hostAddr;
	hostAddr.sin_family = AF_INET;
	hostAddr.sin_port = htons(port);
	hostAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	bind(this->socketID, (SOCKADDR*)&hostAddr, sizeof(SOCKADDR));

	CreateThread(NULL, 0, runRecvThread, this, 0, NULL);
	CreateThread(NULL, 0, runSendThread, this, 0, NULL);
	CreateThread(NULL, 0, runLogcThread, this, 0, NULL);

	return 0;
}

int Peer::Connect(char* ip, unsigned short port, unsigned int max_attempts, unsigned int timeout_ms) {		
	SOCKADDR_IN remote;
	remote.sin_family = AF_INET;
	remote.sin_port = htons(port);
	remote.sin_addr.S_un.S_addr = inet_addr(ip);

	if (connections.find(SA2ULUS(remote)) != connections.end())
		return 0;

	Connection* c = new Connection(remote, this);
	connections.insert( std::pair< ulus, Connection*>( SA2ULUS(remote), c ) );

	Turnkey<bool> connecting;
	c->TryConnecting(max_attempts, timeout_ms, &connecting);

	connecting.Wait();

	return connecting.GetResult();
}

int Peer::ListenForConnection(int max_clients, unsigned int timeout_ms) {
	this->max_clients = max_clients;
	this->connect_timeout = timeout_ms;
	return 1;
}

DataPack* Peer::Receive(bool block, SOCKADDR_IN *sock) {
	game_recv_buffer.Lock();
	if (block) {
		game_recv_buffer.Wait();
	} else {		
		// Decrement the semaphore without blocking.
		if (!game_recv_buffer->empty())		
			game_recv_buffer.Wait();
		// Or exit if the buffer is empty.
		else {		
			game_recv_buffer.Unlock();
			return 0;
		}
	}

	Datagram *dgram = &(game_recv_buffer->front());
	game_recv_buffer->pop();
	game_recv_buffer.Unlock();

	// If we have been given a reference to a 
	// SOCKADDR_IN, fill it.
	if (sock) 
		memcpy(sock, dgram->sock, sizeof(SOCKADDR_IN));		

	return static_cast<DataPack*>(dgram->pack);
}

void Peer::Send(INetPacket *pack, IFilter *filter, bool reliable) 
{
	SOCKADDR_IN addr;
	for (ConnectionTable::iterator it = connections.begin(); 
		it != connections.end(); it++) 
	{
		if (filter->ShouldSend(it->first)) {
			ULUS2SA(it->first, addr);
			Send(pack, &addr, reliable);
		}
	}
}

void Peer::Send(INetPacket *pack, SOCKADDR_IN *remote, bool reliable) 
{	

	ConnectionTable::iterator it = connections.find(SA2ULUS(*remote));
	if(it != connections.end())
	{
		if (dynamic_cast<DataPack*>(pack))
		{
			if(reliable)
			{
				dynamic_cast<DataPack*>(pack)->reliable = true;
				dynamic_cast<DataPack*>(pack)->seq_num = it->second->Seq_Num();
				it->second->TrackRudpPacket(dynamic_cast<DataPack*>(pack), remote);
			}
			else
			{
				dynamic_cast<DataPack*>(pack)->reliable = false;
			}
		}
	}

	Datagram dat;
	dat.reliable = reliable;
	dat.sock = new SOCKADDR_IN(*remote);
	INetPacket* net = static_cast<INetPacket*>(g_NetPackets[ pack->GetType() ].instantiate());
	memcpy(net,pack, g_NetPackets[pack->GetType()].size);
	dat.pack = net;
	Send(&dat);
}
void Peer::Send(Datagram *dat) 
{
	send_buffer.Lock();
	send_buffer->push( *dat );
	send_buffer.Pulse();
	send_buffer.Unlock();
}

int Peer::recvThread(void) {
	char buff[1024];
	SOCKADDR remote;
	int len = sizeof(SOCKADDR);
	while (true) {
		int recv = recvfrom(this->socketID, buff, 1024, 0, &remote, &len);
		INetPacket* packet = PacketEncoder::DecodePacket(buff);
		Datagram dat;
		SOCKADDR_IN *sock = new SOCKADDR_IN( *((SOCKADDR_IN*)&remote));
		dat.sock = sock;
		dat.pack = packet;

		recv_buffer.Lock();
		recv_buffer->push(dat);
		recv_buffer.Pulse();
		recv_buffer.Unlock();
	}
	return 0; 
}

int Peer::sendThread(void) {
	send_buffer.Lock();
	while (true) {
		send_buffer.Wait();

		Datagram data = send_buffer->front();
		send_buffer->pop();
		send_buffer.Unlock();

		char buffer[1024];

		int size = PacketEncoder::EncodePacket( data.pack, buffer, 1024 );

		sendto(this->socketID, buffer, size, 0, (SOCKADDR *)data.sock, sizeof(SOCKADDR));
		send_buffer.Lock();
	}

	return 0;
}

int Peer::logcThread(void) {
	Timer pacing(sleep_time);
	ConnectionTable::iterator it;

	while (true) {
		pacing.Reset();

		recv_buffer.Lock();
		while (recv_buffer->size() >0)
		{
			recv_buffer.Wait();

			Datagram data = recv_buffer->front();
			recv_buffer->pop();

			recv_buffer.Unlock();

			//TODO: not a speed-optimized way of handling packet
			bool handled;
			if (( it = connections.find( SA2ULUS( *data.sock) )) != connections.end())
				handled = it->second->HandlePacket(&data);
			if (!handled)
			{
				// If it's an unprocessed data pack, put it on the buffer and we're done.
				if (dynamic_cast<DataPack*>(data.pack)) {
					game_recv_buffer.Lock();
					game_recv_buffer->push(data);
					game_recv_buffer.Pulse();
					game_recv_buffer.Unlock();
					handled = true;
				}
			}
			if(!handled)
			{
				if (dynamic_cast<ConPack*>(data.pack) && connections.size() < (unsigned int)max_clients) {
					std::pair<ConnectionTable::iterator,bool> it_pair = 
						connections.insert( ConnectionTablePair( SA2ULUS(*data.sock), new Connection(*data.sock, this)));
					it = it_pair.first;
					it->second->HandlePacket(&data);
				}
			}

			recv_buffer.Lock();
		}
		recv_buffer.Unlock();

		for (it = connections.begin(); it != connections.end();) {
			if (!it->second->Update()) 
				connections.erase(it++);
			else
				it++;
		}
		pacing.WaitTillFinished();
	}

	return 0;
}
