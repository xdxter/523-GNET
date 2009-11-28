#pragma once

#include <map>
#include "GNET_Packet.h"

namespace GNET {
	struct DataPack;
	struct IGamePacket {
		virtual ~IGamePacket() {}
		virtual char GetType() const = 0;
	};

	__declspec(deprecated("Deprecated function; you should use Send( (IGamePacket*)packet, ...) instead of Send( CreateDataPack( (IGamePacket*)packet ), ...)."))
	DataPack* CreateDataPack(IGamePacket* packet);

	extern PktRegMap g_GamePackets;	
}