#include <iostream>
#include <map>
#include "winsock2.h"
#include "GNET_Peer.h"

#define MAX_PACKETSIZE 16

using namespace GNET;

DWORD WINAPI runRecvThread(void* param) { return ((Peer*)param)->recvThread(); }
DWORD WINAPI runSendThread(void* param) { return ((Peer*)param)->sendThread(); }
DWORD WINAPI runLogcThread(void* param) { return ((Peer*)param)->logcThread(); }

Peer::Peer()
{	
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

	return 0;
}

int Peer::Connect(char* ip, unsigned short port, unsigned int max_attempts, unsigned int delay) {		
	SOCKADDR_IN remote;
	remote.sin_family = AF_INET;
	remote.sin_port = htons(port);
	remote.sin_addr.S_un.S_addr = inet_addr(ip);
	
	if (connections.find(ADDR(remote)) != connections.end())
		return 0;

	Connection* c = new Connection(remote, this);
	connections.insert( std::pair< ulus, Connection*>( ADDR(remote), c ) );
	c->TryConnect();

	connecting.Wait();

	return c->connect_state == CONNECTED;
}

int Peer::ListenForConnection(int max_clients) {
	this->max_clients = max_clients;

	return 1;
}

DataPack* Peer::Recieve() {
	game_recv_buffer.Lock();
	game_recv_buffer.Wait();
	INetPacket *ret = &(game_recv_buffer->front());
	
	if (dynamic_cast<DataPack*>(ret)) {
		game_recv_buffer->pop();
		game_recv_buffer.Unlock();
		return static_cast<DataPack*>(ret);
	} else {
		game_recv_buffer.Unlock();
		return 0;
	}
}

void Peer::Send(INetPacket *pack, SOCKADDR_IN *remote, bool reliable) 
{	
	Datagram dat;
	dat.reliable = reliable;
	dat.sock = remote;
	dat.pack = pack;
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

		sendto(this->socketID, buffer, size, 0, (SOCKADDR *)&data.sock, sizeof(data.sock));

		send_buffer.Lock();
	}

	return 0;
}

int Peer::logcThread(void) {
	recv_buffer.Lock();
	while (true) {
		recv_buffer.Wait();

		Datagram data = recv_buffer->front();
		recv_buffer->pop();

		recv_buffer.Unlock();

		// If it's a data pack, put it on the buffer and we're done.
		if (dynamic_cast<DataPack*>(data.pack)) {
			game_recv_buffer.Lock();
			game_recv_buffer->push(*(static_cast<DataPack*>(data.pack)));
			game_recv_buffer.Pulse();
			game_recv_buffer.Unlock();
		}

		ConnectionTable::iterator it;
		if (( it = connections.find( ADDR( *data.sock) )) != connections.end())
			it->second->HandlePacket(&data);

		recv_buffer.Lock();
	}

	return 0;
}
