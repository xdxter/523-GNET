#include <map>
#include <queue>
#include "winsock2.h"
#include "GNET_Packet.h"
#include "GNET_GamePacket.h"


#pragma once

namespace GNET {

	struct Datagram {
		bool reliable;
		SOCKADDR_IN *sock;
		INetPacket *pack;
	};

	class Connection;
	class ReliableTracker;

	//common
#define ADDR( sa ) ulus( (sa).sin_addr.S_un.S_addr, (sa).sin_port)
	typedef std::pair<unsigned long, unsigned short> ulus;

	//connection map
	typedef std::pair<ulus, Connection*> ConnectionTablePair;
	typedef std::map<ulus, Connection* > ConnectionTable;
	typedef std::queue<Datagram> DgramBuffer;


	//reliable udp map
	typedef std::pair<ulus, int> ReliableKey;
	typedef std::pair<ReliableKey, ReliableTracker*> ReliableTablePair;
	typedef std::map<ReliableKey, ReliableTracker*> ReliableTable;
	typedef std::map<ReliableKey, ReliableTracker*>::iterator ReliableTableIter;

}