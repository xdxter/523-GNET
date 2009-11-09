#include "GNET_Packet.h"
#include "GNET_Types.h"
#include "Connection.h"
#include "Reliable_UDP.h"
#include "Monitor.h"
#include "Turnkey.h"

#pragma once

namespace GNET {

	class Peer {

	public:
		Peer(unsigned int max_packet_size = 1024);
		virtual ~Peer();

		int Peer::Startup(int max_connections, unsigned short port, int sleep_time = 20);
		int Connect(char* ip, unsigned short port, unsigned int max_attempts = 7, unsigned int delay = 500);
		int ListenForConnection(int max_clients = 1, unsigned int timeout_ms = 500);

		DataPack* Receive(bool should_block, SOCKADDR_IN *sock = 0);
		void Send(INetPacket *pack, SOCKADDR_IN *remote, bool reliable = false);

		int recvThread(void);
		int sendThread(void);
		int logcThread(void);
		SOCKET socketID;

	protected:		
		Turnkey<bool> connecting;
		int connect_timeout;
		Monitor<DgramBuffer> game_recv_buffer;

		friend class Connection;		
		friend class ReliableTracker;
		void Send(Datagram *dat);
	private:
		unsigned int max_connections;
		unsigned int max_clients;
		unsigned int incremental_seq_id;
		int sleep_time;
		unsigned int max_packet_size;

		Monitor<DgramBuffer> send_buffer;
		Monitor<DgramBuffer> recv_buffer;

		ConnectionTable connections;

		HANDLE recv_thread;
		HANDLE send_thread;
	};
}