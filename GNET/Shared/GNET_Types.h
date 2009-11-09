#include <map>
#include <queue>
#include "winsock2.h"
#include "Packets/GNET_Packet.h"
#include "Packets/GNET_GamePacket.h"

#pragma once

#define DEBUG
#ifdef DEBUG
#include <iostream>
#define dd(x) std::cout << x << "\n";//endl;
#else
#define dd(x) 
#endif


namespace GNET {

	class Connection;

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
		bool reliable;
		SOCKADDR_IN *sock;
		INetPacket *pack;
	};

	
	//connection map
	typedef std::pair<ulus, Connection*> ConnectionTablePair;
	typedef std::map<ulus, Connection* > ConnectionTable;
	typedef std::queue<Datagram> DgramBuffer;
	
	typedef std::vector<SOCKADDR_IN> SockBuffer;
}