#pragma once

#include "Packet.h"

enum {
	CLIENT_JOINED,
	CLIENT_MSG
};

#pragma pack(push,1)
	struct ClientJoined : IGamePacket {
		unsigned int uid;
		char team_slot;
		char nick[10];
		PACKET_TYPE(CLIENT_JOINED,ClientJoined)
	};	

	struct ClientMsg : IGamePacket {
		unsigned int uid;
		char msg[140];
		PACKET_TYPE(CLIENT_MSG,ClientMsg)
	};
#pragma pack(pop,1)


void RegisterGamePackets() {
	REGISTER_PACKET(ClientJoined);
	REGISTER_PACKET(ClientMsg);	
}