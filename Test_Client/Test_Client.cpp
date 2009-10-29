// Test_Client.cpp : Defines the entry point for the console application.
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

using namespace GNET;

#pragma pack(push,1)
struct MsgPacket : GNET::IGamePacket {
	char msg[20];
	PACKET_TYPE(0, MsgPacket);
};
#pragma pack(pop)
DWORD WINAPI client(LPVOID lp);

int _tmain(int argc, _TCHAR* argv[])
{
	REGISTER_PACKET(MsgPacket);
	CreateThread(NULL, 0, client, NULL, 0, NULL);
	//Peer *gnet;
	//gnet = new Peer();
	//gnet->Startup(5,5555,50);
	//gnet->Recieve();	//iter error
	//d(received\n);
	getchar();
}

DWORD WINAPI client(LPVOID lp)
{
	Sleep(500);
	Peer *gnet;
	MsgPacket msg;
	strcpy(msg.msg, "This is a message.\n");

	//GNET startup
	gnet = new Peer();
	gnet->Startup(5, 3333, 50);

	//Datapack NOT NECESSARY
	//DataPack dpack;
	//dpack.seq_num=100;
	//dpack.game = static_cast<IGamePacket*>(&msg);

	//sock_addr
	SOCKADDR_IN target;
	target.sin_addr.S_un.S_addr = inet_addr("10.0.1.6");
	target.sin_family = AF_INET;
	target.sin_port = htons(5555);

	char buffer[10];
	//sendto(gnet->socketID, buffer, 10, 0, (SOCKADDR *)&target, sizeof(SOCKADDR));
	dd(sizeof(msg), d);
	IGamePacket *ii = static_cast<IGamePacket*>(&msg);

	gnet->Send(CreateDataPack(&msg), &target, false);
	d(sent...\n);
	getchar();
	return 1;
}

