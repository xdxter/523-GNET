// Test_Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "GNET_GamePacket.h"
#include "GNET_Packet.h"
#include "GNET_Peer.h"

using namespace GNET;

#pragma pack(push,1)
struct MsgPacket : GNET::IGamePacket {
	char msg[30];
	PACKET_TYPE(8, MsgPacket);
};
#pragma pack(pop)

#define LISTEN_PORT 3333
#define SERVER_PORT 4444
#define SERVER_ADDY "127.0.0.1"

int _tmain(int argc, _TCHAR* argv[])
{
	REGISTER_PACKET(MsgPacket, GNET::g_GamePackets);
	
	//GNET startup
	Peer *gnet;
	gnet = new Peer();
	gnet->Startup(1, LISTEN_PORT, 50);

	// Connect to server
	bool connected = gnet->Connect(SERVER_ADDY, SERVER_PORT, 3);
	//bool connected = true;
	if (!connected) {
		printf("Failed to connect to server.\n");
	} 
	else {
		printf("Connected to server succesfully.\n");
	
		// Send a data packet
		MsgPacket msg;
		strcpy(msg.msg, "A message from client..");

		SOCKADDR_IN target;
		target.sin_addr.S_un.S_addr = inet_addr(SERVER_ADDY);
		target.sin_family = AF_INET;
		target.sin_port = htons(SERVER_PORT);

		gnet->Send(CreateDataPack(&msg),&target, true);
		gnet->Send(CreateDataPack(&msg),&target, true);

		// Receive a packet. Passing true in means that the call will block until one is received.
		DataPack * p = gnet->Receive(true);
		printf("Message Received: %s\n", static_cast<MsgPacket*>(p->game)->msg);
	}
	getchar();
	return 1;
}

