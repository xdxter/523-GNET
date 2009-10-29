#include "GNET_Packet.h"
#include "GNET_Types.h"
#include "Connection.h"
#include "Monitor.h"
#include "Turnkey.h"

#pragma once

namespace GNET {
#define ADDR( sa ) ulus( (sa).sin_addr.S_un.S_addr, (sa).sin_port)
	typedef std::pair<unsigned long, unsigned short> ulus;
	typedef std::pair<ulus, Connection*> ConnectionTablePair;
	typedef std::map<ulus, Connection* > ConnectionTable;
	typedef std::queue<Datagram> DgramBuffer;

	class Peer {

	public:
		Peer();
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

		friend class Connection;		
		void Send(Datagram *dat);
	private:
		int max_connections;
		int max_clients;
		int sleep_time;

		Monitor<DgramBuffer> send_buffer;
		Monitor<DgramBuffer> recv_buffer;
		
		Monitor<DgramBuffer> game_recv_buffer;

		ConnectionTable connections;


		HANDLE recv_thread;
		HANDLE send_thread;
	};
}