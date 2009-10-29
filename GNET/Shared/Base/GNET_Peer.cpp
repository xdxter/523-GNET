#include <iostream>
#include <map>
#include "winsock2.h"
#include "GNET_Peer.h"
#include "Timer.h"

#define MAX_PACKETSIZE 16

using namespace GNET;

DWORD WINAPI runRecvThread(void* param) { return ((Peer*)param)->recvThread(); }
DWORD WINAPI runSendThread(void* param) { return ((Peer*)param)->sendThread(); }
DWORD WINAPI runLogcThread(void* param) { return ((Peer*)param)->logcThread(); }

Peer::Peer()
{	
	PacketEncoder::RegisterNetPackets();
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
	
	if (connections.find(ADDR(remote)) != connections.end())
		return 0;

	Connection* c = new Connection(remote, this);
	connections.insert( std::pair< ulus, Connection*>( ADDR(remote), c ) );
	c->TryConnecting(max_attempts, timeout_ms);

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
		memcpy(sock, dgram->pack, sizeof(SOCKADDR_IN));		

	return static_cast<DataPack*>(dgram->pack);
}

void Peer::Send(INetPacket *pack, SOCKADDR_IN *remote, bool reliable) 
{	
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

		char buffer[100];

		int size = PacketEncoder::EncodePacket( data.pack, buffer, 100 );

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

			// If it's a data pack, put it on the buffer and we're done.
			if (dynamic_cast<DataPack*>(data.pack)) {
				game_recv_buffer.Lock();
				game_recv_buffer->push(data);
				game_recv_buffer.Pulse();
				game_recv_buffer.Unlock();
			}

			if (( it = connections.find( ADDR( *data.sock) )) != connections.end())
				it->second->HandlePacket(&data);
			else if (dynamic_cast<ConPack*>(data.pack) && connections.size() < max_clients) {
				std::pair<ConnectionTable::iterator,bool> it_pair = 
					connections.insert( ConnectionTablePair( ADDR(*data.sock), new Connection(*data.sock, this)));
				it = it_pair.first;
				it->second->HandlePacket(&data);
			}

			recv_buffer.Lock();
		}
		recv_buffer.Unlock();

		for (it = connections.begin(); it != connections.end(); it++) {
			it->second->Update();
		}
		pacing.WaitTillFinished();
	}

	return 0;
}
