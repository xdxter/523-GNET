// Test_Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <math.h>
#include "Packets/GNET_GamePacket.h"
#include "Packets/GNET_Packet.h"
#include "GNET_Peer.h"

using namespace GNET;

#pragma pack(push,1)
struct MsgPacket : GNET::IGamePacket {
	char msg[20];
	PACKET_TYPE(0, MsgPacket);
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
	gnet->Startup(1, (rand() % 1000) + 4000, 7);

	// Connect to server
	bool connected = gnet->Connect(SERVER_ADDY, SERVER_PORT, 3);
	if (!connected) {
		printf("Failed to connect to server.\n");
	} else {
		printf("Connected to server succesfully.\n");
	
		// Send a data packet
		MsgPacket msg;
		strcpy(msg.msg, "This is a message.");

		SOCKADDR_IN target;
		target.sin_addr.S_un.S_addr = inet_addr(SERVER_ADDY);
		target.sin_family = AF_INET;
		target.sin_port = htons(SERVER_PORT);

		//test Simulation of Dropping Packets
		/*while(true)
		{
			gnet->NSimulatorSend(CreateDataPack(&msg),&target);
		}*/



		gnet->Send( CreateDataPack(&msg), &target);


		//test reliable udp
		for(int i = 0; i<1; i++)
			gnet->Send( &msg ,&target, RELIABLE | SEQUENCED);

		while (true) {
		// Receive a packet. Passing true in means that the call will block until one is received.
			DataPack * p = gnet->Receive(true);
			printf("Message Received: %s\n", static_cast<MsgPacket*>(p->game)->msg);
		}
	}

	getchar();
	return 1;
}

