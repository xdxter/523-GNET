// Test_Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "GNET_GamePacket.h"
#include "GNET_Packet.h"
#include "GNET_Peer.h"
#include "Timer.h"

#define __debug__ __tool__
#define d(x) printf(#x);
#define dd(x, fmt) printf("=====DEBUG====> Line %u: %s=" "%" #fmt "\n" , __LINE__, #x, x)
#define ddd(extra, x, fmt) printf(#extra "     =====DEBUG====> Line %u: %s=" "%" #fmt "\n" , __LINE__, #x, x)
#define dddd() printf("*************DEBUG*************\n")

#pragma pack(push,1)
struct MsgPacket : GNET::IGamePacket {
	char msg[20];
	PACKET_TYPE(0, MsgPacket);
};
#pragma pack(pop)

using namespace GNET;

int _tmain(int argc, _TCHAR* argv[])
{
	REGISTER_PACKET(MsgPacket, GNET::g_GamePackets);
	Peer *gnet;
	gnet = new Peer();
	gnet->Startup(5,4444,50);
	//gnet->ListenForConnection(1);

	while (true) {
		Datagram *dat = gnet->Receive(true);
		DataPack *dp = dynamic_cast<DataPack*>(dat->pack);
		if (dp) {
			ddd("Received message:",dynamic_cast<MsgPacket*>(dp->game)->msg,s);
			MsgPacket m;
			strcpy(m.msg,"CANDLEJACK SAYS H-");
			gnet->Send(CreateDataPack(&m), dat->sock, 1);
		}
	}
	
	return 0;
}

