// Test_Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "GNET_GamePacket.h"
#include "GNET_Packet.h"
#include "GNET_Peer.h"

#pragma pack(push,1)
struct MsgPacket : GNET::IGamePacket {
	char msg[20];
	PACKET_TYPE(0, MsgPacket);
};
#pragma pack(pop)
using namespace GNET;
DWORD WINAPI client(LPVOID lp);

int _tmain(int argc, _TCHAR* argv[])
{
	REGISTER_PACKET(MsgPacket);
	CreateThread(NULL, 0, client, NULL, 0, NULL);
	Peer *gnet;
	gnet = new Peer();
	gnet->Startup(5,5555,50);
	//gnet->Recieve();	//iter error
}

DWORD WINAPI client(LPVOID lp)
{
	Sleep(500);
	Peer *gnet;
	MsgPacket msg;
	strcpy(msg.msg, "This is a message.\n");

	bool is_client = true;
	if (is_client) {
		gnet = new Peer();
		gnet->Startup(5, 3333, 50);
		gnet->Connect("127.0.0.1", 5555, 5, 500);
		DataPack data;
		data.seq_num=100;
		data.game = static_cast<IGamePacket*>(&msg);
	}
	return 1;
}

