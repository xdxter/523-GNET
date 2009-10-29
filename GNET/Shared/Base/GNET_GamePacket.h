#pragma once

#include <map>
#include "GNET_Packet.h"

namespace GNET {
	struct DataPack;
	struct IGamePacket {
		virtual ~IGamePacket() {}
		virtual char GetType() = 0;
	};

	DataPack* CreateDataPack(IGamePacket* packet);
	extern PktRegMap g_GamePackets;	
}