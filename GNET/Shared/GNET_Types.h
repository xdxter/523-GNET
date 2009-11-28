#include <map>
#include <queue>
#include "winsock2.h"
#include "Packets/GNET_Packet.h"
#include "Packets/GNET_GamePacket.h"

#pragma once

#ifdef _DEBUG
#include <iostream>
#define DBG_PRINT(x) std::cout << x << "\n";
#define SOCK_PRNT(x) inet_ntoa((x).sin_addr) << ":" << ntohs((x).sin_port)
#else
#define DBG_PRINT(x) 
#define SOCK_PRNT(x) 
#endif



namespace GNET {

	class Connection;
	struct RudpItem;

	///Commonly used Macros
	#define SA2ULUS( sa ) ulus( (sa).sin_addr.S_un.S_addr, (sa).sin_port)
	#define ULUS2SA( pair, socket ) 	{							\
		socket.sin_family = AF_INET;							\
		socket.sin_addr.S_un.S_addr = pair.first;				\
		socket.sin_port = pair.second; }		
	typedef std::pair<unsigned long, unsigned short> ulus;

	///The function in the structure returns true if the data is to be sent to a particular client 
	///and returns false if not
	struct IFilter {
		virtual bool ShouldSend(const ulus &remote) = 0;
	};

	//Structure used by the server for filtering - to send the data to everyone except a particular client
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
	
	///A structure which contains the SOCKADDR_IN and the actual packet.
	struct Datagram {
		char flags;
		SOCKADDR_IN *sock;
		INetPacket *pack;

		Datagram() {
			flags = 0;
			sock = 0;
			pack = 0;
		}
		Datagram(const Datagram& other) {
			flags = other.flags;
			sock = new SOCKADDR_IN(*(other.sock));
			pack = PacketEncoder::CreateCopy(*(other.pack));
		}
		~Datagram() {
			if (sock)
				delete sock;
			if (pack)
				delete pack;
		}
		Datagram& operator= (const Datagram& other) {
			if (this != &other) // protect against invalid self-assignment
			{
				flags = other.flags;
				if (sock) delete sock;
				sock = new SOCKADDR_IN(*(other.sock));	
				if (pack) delete pack;
				pack = PacketEncoder::CreateCopy(*(other.pack));
			}
			return *this;
		}

	};

	///Enum of all the send options the GNET provides
	enum SendFlags
	{
		NONE = 0,
		RELIABLE = 1,
		SEQUENCED = 2,
		AGGREGATED = 4,
		COMPRESSED = 8
	};
	
	///A map to handle the connections
	typedef std::pair<ulus, Connection*> ConnectionTablePair;
	typedef std::map<ulus, Connection* > ConnectionTable;
	typedef std::queue<Datagram> DgramBuffer;
	typedef std::queue<SOCKADDR_IN> SockBuffer;

}