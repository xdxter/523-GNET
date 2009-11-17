#include <map>
#include "winsock2.h"
#include "GNET_Peer.h"
#include "GNET_Compression.h"

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

int Peer::ListenForConnection(int max_clients, unsigned int timeout_ms, bool discovery_mode) {
	this->max_clients = max_clients;
	this->connect_timeout = timeout_ms;
	this->discovery_mode = discovery_mode;

	//allow broadcasting
	if(discovery_mode)
	{
	    bool bOptVal = true;
	    int bOptLen = sizeof(bool);
	    int iOptLen = sizeof(int);
		setsockopt(this->socketID, SOL_SOCKET, SO_BROADCAST, (char*)&bOptVal, bOptLen); 
	}
	return 1;
}

//Receive Taking packets only according to delay
DataPack* Peer::PackLossSimulatorReceive(bool block, SOCKADDR_IN *sock, int delay)
{
	
	Timer check_time(delay);
	check_time.WaitTillFinished();
	
			DataPack *dp = Receive(block, sock);
			return dp;
	
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

void Peer::DiscoverServer(int port)
{
	SOCKADDR_IN target;
	target.sin_addr.S_un.S_addr = htonl(INADDR_BROADCAST);	//send by broadcast
	target.sin_family = AF_INET;
	target.sin_port = htons(port);

	DhcpPack pack;
	pack.instigator = true;

	Datagram dgram;
	dgram.pack = DhcpPack;
	dgram.sock = target;
	//send broadcast message
	Send(&dgram);
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


//Added a function to drop the packets which the user gives to send
static int delay1;
static int delay2;

void Peer::NSimulatorSend(INetPacket *pack, SOCKADDR_IN *remote, bool reliable)
{

	delay1 = rand()%60000+1;
	delay2 = rand()%10+1;
	
	if(delay1==999 && delay2==4)
	{
		Send(pack,remote);
	}
	//Randomly Dropping Packets


}

void Peer::Send(INetPacket *pack, SOCKADDR_IN *remote, char FLAGS)
{	

	ConnectionTable::iterator it = connections.find(SA2ULUS(*remote));
	if(it != connections.end())
	{
		if (dynamic_cast<DataPack*>(pack))
		{
			dynamic_cast<DataPack*>(pack)->flags = FLAGS;//setting datapack flag
			if(FLAGS & SEQUENCED || FLAGS & RELIABLE)
			{
				dynamic_cast<DataPack*>(pack)->seq_num = it->second->Seq_Num();
				if(FLAGS & RELIABLE)
				{
					it->second->TrackRudpPacket(dynamic_cast<DataPack*>(pack), remote);
				}
			}
		}
	}

	Datagram dat;
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
	compression dd;
	int len = sizeof(SOCKADDR);
	while (true) {
		int recv = recvfrom(this->socketID, buff, 1024, 0, &remote, &len);
		////add compression
		//char *output=new char[1024];
		//int outputsize=1024;
		//char* ptr=buff;
		//if(*ptr==1)	//compressed
		//{
		//	int aa=dd.decomp(ptr+1,output,outputsize);
		//	if (aa==-1)	//output is not large enough
		//	{
		//		delete output;
		//		output= new char[outputsize];
		//		aa=dd.decomp(ptr+1,output,outputsize);
		//	}
		//}
		//else		//uncompressed
		//{
		//	memcpy(output,buff+1,1023);
		//}
		////finish
		INetPacket* packet = PacketEncoder::DecodePacket(buff);
		Datagram dat;
		SOCKADDR_IN *sock = new SOCKADDR_IN( *((SOCKADDR_IN*)&remote));
		dat.sock = sock;
		dat.pack = packet;

		recv_buffer.Lock();
		recv_buffer->push(dat);
		recv_buffer.Pulse();
		recv_buffer.Unlock();
		//delete output;
	}
	return 0; 
}

int Peer::sendThread(void) {
	send_buffer.Lock();
	compression dd;
	while (true) {
		send_buffer.Wait();

		Datagram data = send_buffer->front();
		send_buffer->pop();
		send_buffer.Unlock();

		char buffer[1024];

		int size = PacketEncoder::EncodePacket( data.pack, buffer, 1024 );

		//char *output=new char[1024];
		//int outputsize=1024;
		//int length;
		//char* ptr=buffer;
		////add compression
		//length=dd.comp(buffer,size,output,outputsize);
		//if (length==-1)	//output is not enough
		//{
		//	delete output;
		//	output= new char[outputsize];
		//	length=dd.comp(buffer,size,output,outputsize);
		//}
		//if(length<size)	//worth compress
		//{
		//	length++;
		//	memcpy(output+1,output,length);
		//	*output=1;
		//}
		//else			//don't compress
		//{
		//	*output=0;
		//	length=size+1;
		//	if(length>1024) length=1024;
		//	memcpy(output+1,ptr,length);
		//	
		//}
		//finish	
		sendto(this->socketID, buffer, size, 0, (SOCKADDR *)data.sock, sizeof(SOCKADDR));
		send_buffer.Lock();
		//delete output;
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

			//pRemote(*data.sock);
			printf("received something...... from        ");pRemote(*data.sock);printf("\n");


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
