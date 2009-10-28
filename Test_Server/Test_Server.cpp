// Test_Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "GNET_GamePacket.h"
#include "GNET_Packet.h"
#include "GNET_Peer.h"

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
	REGISTER_PACKET(MsgPacket);
	Peer *gnet;
	gnet = new Peer();
	gnet->Startup(5,5555,50);
	getchar();
	return 0;
}

