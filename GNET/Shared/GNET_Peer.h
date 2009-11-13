#include "GNET_Types.h"
#include "Connections/Connection.h"
#include "Base/Monitor.h"
#include "Base/Turnkey.h"

#pragma once


enum SendOptions
{
	NONE = 0,
	RELIABLE = 1,
	SEQUENCED = 2,
	AGGREGATED = 4
};

namespace GNET {

	class Peer {

	public:
		Peer(unsigned int max_packet_size = 1024);
		virtual ~Peer();

		int Startup(int max_connections, unsigned short port, int sleep_time = 20);
		bool Connect(char* ip, unsigned short port, unsigned int max_attempts = 7, unsigned int delay = 500);
		int ListenForConnection(int max_clients = 1, unsigned int timeout_ms = 500);

		DataPack* Receive(bool should_block, SOCKADDR_IN *sock = 0);
		void Send(INetPacket *pack, IFilter *filter, bool reliable = false);
		void Send(INetPacket *pack, SOCKADDR_IN *remote, char flag = NONE);
		SOCKADDR_IN* ClientEntered(bool should_block = false);
		SOCKADDR_IN* ClientExited(bool should_block = false);

		//test funcs by aapte
		void NSimulatorSend(INetPacket *pack, SOCKADDR_IN *remote, bool reliable = false);
		DataPack* PackLossSimulatorReceive(bool should_block, SOCKADDR_IN *sock = 0, int delay = 0);

		int recvThread(void);
		int sendThread(void);
		int logcThread(void);
		SOCKET socketID;

	protected:		
		friend class Connection;
		friend class ReliableTracker;

		int connect_timeout;
		Monitor<SockBuffer>	connection_events; 
		Monitor<SockBuffer> disconnect_events;

		void Send(Datagram *dat);

	private:
		int max_connections;
		int max_clients;
		int sleep_time;
		unsigned int max_packet_size;

		Monitor<DgramBuffer> send_buffer;
		Monitor<DgramBuffer> recv_buffer;
		
		Monitor<DgramBuffer> game_recv_buffer;

		ConnectionTable connections;


		HANDLE recv_thread;
		HANDLE send_thread;
	};
}