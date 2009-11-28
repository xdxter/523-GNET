#include <map>
#include "winsock2.h"
#include "GNET_Peer.h"
#include "Packets/Compression.h"

#define MAX_PACKETSIZE 16

using namespace GNET;

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

	this->socketID = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (this->socketID == INVALID_SOCKET) {
		WSACleanup();
		return SOCK_CREATE_FAILED;
	}

	SOCKADDR_IN hostAddr;
	hostAddr.sin_family = AF_INET;
	hostAddr.sin_port = htons(port);
	hostAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	bind(this->socketID, (SOCKADDR*)&hostAddr, sizeof(SOCKADDR));

	CreateThread(NULL, 0, Peer::runRecvThread, this, 0, NULL);
	CreateThread(NULL, 0, Peer::runSendThread, this, 0, NULL);
	CreateThread(NULL, 0, Peer::runLogcThread, this, 0, NULL);

	return 0;
}

bool Peer::Connect(char* ip, unsigned short port, unsigned int max_attempts, unsigned int timeout_ms) {		
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

	DBG_PRINT("GNET: Waiting for connection.");
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

void Peer::Send(IGamePacket *pack, IFilter *filter, char flags) 
{
	DataPack *dat = new DataPack;
	dat->game = PacketEncoder::CreateCopy(*pack);
	Send(dat, filter, flags);
}

void Peer::Send(IGamePacket *pack, SOCKADDR_IN *remote, char flags) 
{
	DataPack *dat = new DataPack;
	dat->game = PacketEncoder::CreateCopy(*pack);
	Send(dat, remote, flags);
}

void Peer::Send(INetPacket *pack, IFilter *filter, char flags) 
{
	SOCKADDR_IN addr;
	for (ConnectionTable::iterator it = connections.begin(); 
		it != connections.end(); it++) 
	{
		if (filter->ShouldSend(it->first)) {
			ULUS2SA(it->first, addr);
			Send(pack, &addr, flags);
		}
	}
}

void Peer::Send(INetPacket *pack, SOCKADDR_IN *remote, char flags)
{	
	// Assemble a Datagram to hold this info
	Datagram* dgram = new Datagram;
	dgram->flags = flags;
	dgram->sock = new SOCKADDR_IN(*remote);
	dgram->pack = PacketEncoder::CreateCopy(*pack);	

	// Would our connection like to handle sending instead?
	ConnectionTable::iterator it = connections.find(SA2ULUS(*remote));
	if(it != connections.end())
	{
		bool send_handled = it->second->SendingPacket(dgram);
		if (send_handled) {
			delete dgram;			
			return;
		}
	}

	// Time to send!
	Send(dgram);
}
void Peer::Send(Datagram *dgram) 
{
	send_buffer.Lock();
	send_buffer->push( *dgram );
	send_buffer.Pulse();
	send_buffer.Unlock();
}


SOCKADDR_IN* Peer::ClientEntered(bool should_block) {
	connection_events.Lock();
	if (should_block) {
		connection_events.Wait();
	} else {		
		// Decrement the semaphore without blocking.
		if (!connection_events->empty())		
			connection_events.Wait();
		// Or exit if the buffer is empty.
		else {		
			connection_events.Unlock();
			return 0;
		}
	}
	SOCKADDR_IN *sock = &(connection_events->front());
	connection_events->pop();
	connection_events.Unlock();

	return sock;
}

SOCKADDR_IN* Peer::ClientExited(bool should_block) {
	disconnect_events.Lock();
	if (should_block) {
		disconnect_events.Wait();
	} else {		
		// Decrement the semaphore without blocking.
		if (!disconnect_events->empty())		
			disconnect_events.Wait();
		// Or exit if the buffer is empty.
		else {		
			disconnect_events.Unlock();
			return 0;
		}
	}
	SOCKADDR_IN *sock = &(disconnect_events->front());
	disconnect_events->pop();
	disconnect_events.Unlock();

	return sock;
}

int Peer::recvThread(void) {
	// Packet Decoding Vars
	INetPacket* packet;
	char* buff = new char[max_packet_size];
	SOCKADDR remote;
	int len = sizeof(SOCKADDR);
	
	// Compression Vars
	Compression compress;
	char* compressed_buff = new char[max_packet_size - 1]; 
	
	// Thread Loop
	while (true) {
	//	remote = 0;
		len = sizeof(SOCKADDR);
		int recv = recvfrom(this->socketID, buff, max_packet_size, 0, &remote, &len);	
		if (recv==0) {
			DBG_PRINT("Socket has gracefully closed.");
			break;  // socket has been gracefully closed.
		}
		if (recv==-1) {
			int error = WSAGetLastError();			
			if (error == 10054) {
				//PRINT_SOCKADDR(*((SOCKADDR_IN*)&remote));
				continue; // Connection reset by peer. We should actually handle this.
			}
			DBG_PRINT("RecvThread WSA Error #" << WSAGetLastError());
			break; // error...
		}

		if( (*buff) & COMPRESSED ) {

			memcpy(compressed_buff, buff+1, recv-1);
			recv = compress.Decompress(compressed_buff, recv - 1, buff + 1, max_packet_size - 1);	
			if (recv == -1)	
				continue; // If the packet is too big for our buffer, just ignore it.
			recv++;	// account for flag char at start...
		}
		
		packet = PacketEncoder::DecodePacket(buff, 1);

		// Assemble datagram
		Datagram dat;
		SOCKADDR_IN *sock = new SOCKADDR_IN( *((SOCKADDR_IN*)&remote));
		dat.flags = *buff; // First byte is flag byte.
		dat.sock = sock;
		dat.pack = packet;
		
		// Stick the datagram into the buffer
		recv_buffer.Lock();
		recv_buffer->push(dat);
		recv_buffer.Pulse();
		recv_buffer.Unlock();

	}

	delete [] buff;
	delete [] compressed_buff;
	return 0; 
}

int Peer::sendThread(void) {
	// Initializing variables outside of the loop for efficiency.
	Compression compress;
	char* buff = new char[max_packet_size];
	char* uncompressed_buff = new char[max_packet_size]; // this is only used when compressing.
	Datagram data;
	
	while (true) {
		// Grab a packet off the send buffer
		send_buffer.Lock();	
		send_buffer.Wait();
		data = send_buffer->front();
		send_buffer->pop();
		send_buffer.Unlock();
		
		// Decode packet
		buff[0] = data.flags;
		int size = PacketEncoder::EncodePacket( data.pack, buff, max_packet_size, 1 );

		// Run compression & security, if required.
		if (data.flags & COMPRESSED) { 
			memcpy(uncompressed_buff,buff,size);

			size = compress.Compress(uncompressed_buff+1, size - 1, buff, max_packet_size);
			if (size==-1) // too big.
				continue;	// so ignore.
			size++; // account for the char flag at the start.
		}
			
		// Send it
		sendto(this->socketID, buff, size, 0, (SOCKADDR *)data.sock, sizeof(SOCKADDR));	
	}

	delete [] buff;
	delete [] uncompressed_buff;
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


			it = connections.find(SA2ULUS(*data.sock));

			if (it != connections.end()) {
				if (!it->second->HandlePacket(&data)) {
					if (dynamic_cast<DataPack*>(data.pack)) {
						game_recv_buffer.Lock();
						game_recv_buffer->push(data);
						game_recv_buffer.Pulse();
						game_recv_buffer.Unlock();
					}
				}
			} else if (dynamic_cast<ConPack*>(data.pack) && connections.size() < (unsigned int)max_clients) {
					std::pair<ConnectionTable::iterator,bool> it_pair = 
						connections.insert( ConnectionTablePair( SA2ULUS(*data.sock), new Connection(*data.sock, this)));
					if (it_pair.second) 
						it_pair.first->second->HandlePacket(&data);
			}

			recv_buffer.Lock();
		}
		recv_buffer.Unlock();

		for (it = connections.begin(); it != connections.end();) {
			if (!it->second->Update()) 
				connections.erase(it++);	// Post increment is required here to prevent errors, don't change.
			else							// ...
				it++;						// just don't do it.
		}
		pacing.WaitTillFinished();
	}

	return 0;
}
