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

}