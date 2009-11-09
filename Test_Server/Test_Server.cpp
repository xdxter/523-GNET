// Test_Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "Packets/GNET_GamePacket.h"
#include "Packets/GNET_Packet.h"
#include "GNET_Peer.h"


#pragma pack(push,1)
struct MsgPacket : GNET::IGamePacket {
	char msg[20];
	PACKET_TYPE(0, MsgPacket);
};
#pragma pack(pop)

int _tmain(int argc, _TCHAR* argv[])
{
	REGISTER_PACKET(MsgPacket, GNET::g_GamePackets);
	GNET::Peer *gnet;
	gnet = new GNET::Peer();
	gnet->Startup(5,4444,50);
	gnet->ListenForConnection(4);

	SOCKADDR_IN remote;
	while (true) {		
		GNET::DataPack *dp = gnet->Receive(true, &remote);

		if (dp) {
			printf("Received message from %d:%d -- %s", remote.sin_addr.S_un.S_addr, remote.sin_port,
				static_cast<MsgPacket*>(dp->game)->msg);

			// Reply to user
			MsgPacket m;
			strcpy(m.msg,"CANDLEJACK SAYS H-");
			gnet->Send(CreateDataPack(&m), &remote, 1);

			// Broadcast message to others
			GNET::FilterEveryoneBut filter(&remote);
			gnet->Send(dp, &filter, 1);
		}
	}
	
	return 0;
}

