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
	gnet->Recieve();	//iter error
	d(received\n);
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
		//gnet->Connect("127.0.0.1", 5555, 7, 500);
		//Datapack
		DataPack dpack;
		dpack.seq_num=100;
		dpack.game = static_cast<IGamePacket*>(&msg);

		//sock_addr
		SOCKADDR_IN target;
		target.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
		target.sin_family = AF_INET;
		target.sin_port = htons(5555);

		gnet->Send(static_cast<INetPacket*>(&dpack), &target, false);
		d(sent\n);
	}
	return 1;
}

