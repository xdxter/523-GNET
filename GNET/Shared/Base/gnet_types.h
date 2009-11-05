#include <map>
#include <queue>
#include "winsock2.h"
#include "GNET_Packet.h"
#include "GNET_GamePacket.h"


#pragma once

namespace GNET {

	class Connection;
	class ReliableTracker;
	class Peer;
	struct RudpItem;

	struct Datagram {
		bool reliable;
		SOCKADDR_IN *sock;
		INetPacket *pack;
	};


	//common
#define ADDR( sa ) ulus( (sa).sin_addr.S_un.S_addr, (sa).sin_port)
	typedef std::pair<unsigned long, unsigned short> ulus;

	//connection map
	typedef std::pair<ulus, Connection*> ConnectionTablePair;
	typedef std::map<ulus, Connection* > ConnectionTable;
	typedef std::queue<Datagram> DgramBuffer;

	//reliable udp map
	//typedef std::pair<bool, unsigned int> boui;
	typedef std::pair<ulus, int> ReliableKey;
	typedef std::pair<ReliableKey, RudpItem*> ReliableTablePair;
	typedef std::map<ReliableKey, RudpItem*> ReliableTable;
	typedef std::map<ReliableKey, RudpItem*>::iterator ReliableTableIter;

}