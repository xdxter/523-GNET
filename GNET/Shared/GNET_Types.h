#include <map>
#include <queue>
#include "winsock2.h"
#include "Packets/GNET_Packet.h"
#include "Packets/GNET_GamePacket.h"

#pragma once

#define DEBUG
#ifdef DEBUG
#include <iostream>
#define dd(x) std::cout << x << "\n";
#define pRemote(x) printf("Received from %s:%d ==> ", inet_ntoa((x).sin_addr), ntohs((x).sin_port))
#else
#define dd(x) 
#define pSource(x) 
#endif


namespace GNET {

	class Connection;
	struct RudpItem;

	//common
	#define SA2ULUS( sa ) ulus( (sa).sin_addr.S_un.S_addr, (sa).sin_port)
	#define ULUS2SA( pair, socket ) 	{							\
		socket.sin_family = AF_INET;							\
		socket.sin_addr.S_un.S_addr = pair.first;				\
		socket.sin_port = pair.second; }		
	typedef std::pair<unsigned long, unsigned short> ulus;

	struct IFilter {
		virtual bool ShouldSend(const ulus &remote) = 0;
	};
	struct FilterEveryoneBut : IFilter {
	private:
		ulus source;
	public:
		FilterEveryoneBut(SOCKADDR_IN *source) {
			this->source = SA2ULUS(*source);
		}
		bool ShouldSend(const ulus &remote) {
			return (source != remote);
		}
	};
	struct Datagram {
		SOCKADDR_IN *sock;
		INetPacket *pack;
	};

	
	//connection map
	typedef std::pair<ulus, Connection*> ConnectionTablePair;
	typedef std::map<ulus, Connection* > ConnectionTable;
	typedef std::queue<Datagram> DgramBuffer;
	typedef std::queue<SOCKADDR_IN> SockBuffer;


	//reliable udp map
	typedef std::pair<int, RudpItem*> ReliableTablePair;
	typedef std::map<int, RudpItem*> ReliableTable;
	typedef std::map<int, RudpItem*>::iterator ReliableTableIter;
}