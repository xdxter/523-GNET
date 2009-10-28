#pragma once

#include <map>
#include "GNET_Packet.h"

namespace GNET {
	struct DataPack;
	struct IGamePacket {
		virtual ~IGamePacket() {}
		virtual char GetType() = 0;
	};

	struct GamePktReg {	
		int size;
		IGamePacket* (*instantiate) (void);
	};

	#define REGISTER_PACKET(T)	{								\
		GNET::GamePktReg pkt_data; T t;							\
		pkt_data.size = sizeof(T);								\
		pkt_data.instantiate = t.Instantiate;					\
		GNET::g_GamePackets.insert(								\
			std::pair<char,GNET::GamePktReg>(t.GetType(), pkt_data)); }

	#define PACKET_TYPE(num,x)								\
		char GetType() { return num; }						\
		static IGamePacket* Instantiate() { return new x; }


	extern std::map<char,GamePktReg> g_GamePackets;
	
	DataPack* CreateDataPack(IGamePacket* packet);
}