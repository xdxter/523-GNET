#include "winsock2.h"
#include <iostream>
#include "GNET_Peer.h"

#define MAX_PACKETSIZE 16

using namespace GNET;

Peer::Peer()
{	
}

int Peer::Startup()
{
	WSADATA WsaDat;

	if (WSAStartup(MAKEWORD(2, 0), &WsaDat) != 0) {
		cout << "WSA Initialization failed!!\r\n";
		WSACleanup();
		system("PAUSE");
		return 0;
	}

	this->socketID = socket(AF_INET, SOCK_DGRAM, 0);
	if (this->socketID == INVALID_SOCKET) {
		cout << "Socket creation failed.\r\n";
		WSACleanup();
		system("PAUSE");
		return 0;
	}

	return 1;
}


void Peer::ListenOnPort(SysAddress &sock_desc)
{
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(sock_desc->port);
	sockAddr.sin_addr.S_un.S_addr = sock_desc->binaryAddress;
	
	bind(socketID, (sockaddr*)sockaddr, sizeof(sockaddr));
}

SysAddress Peer::getSysAddress(char *ip, int port)
{
	SysAddress addr;
	addr.binaryAddress=inet_addr(ip);
	addr.port=(short)port;

	return addr;
}

int Peer::SendTo(Packet packet,SysAddress sendToSA)
{
	sockaddr_in sendTo;
	sockaddr &sendToCast = (sockaddr&)sendTo;

	sendTo.sin_family = AF_INET;
	sendTo.sin_port = htons(sendToSA.portNumber);

	in_addr ipaddr;
	ipaddr.S_un.S_addr = sendToSA.binaryAddress;
	struct hostent *hp = gethostbyaddr((char*)&ipaddr,4,AF_INET);
	memcpy((char*)&sendTo.sin_addr,(char*)hp->h_addr_list[0],hp->h_length);

	////////////////////////
	//Implement Encryption of packets here
	//Call private fucntion to return encrypted data packet
	////////////////////////

	//////////////////////////
	//Implement Compression of packets here
	//Call private function and pass data address as parameter and return compressed data packets
	//////////////////////////

	//////////////////////////
	// if RELIABLE UDP
	// then add data packet to the Send list struture and wait for ACK
	//////////////////////////
   char buff[1];
   buff[0] = 'a';
   sendto(socketID,packet.data,1,0, &sendToCast, sizeof(sendTo));
	return 1;
}

int runThreadHandler(void* param)
{
	Peer *peer = (Peer*)param;
	return peer->runThread();
}

int Peer::runThread()
{
	char recbuffer[MAX_PACKETSIZE];
	int size = sizeof(sockAddr);
	
	sockaddr &sockAddrCast = (sockaddr&)sockAddr;
	recvfrom(socketID,recbuffer,MAX_PACKETSIZE,0,&sockAddrCast,&size);
	
	//////////////////////////
	//Implement Decompression of packets here
	//Call private function and pass data packets as parameter and return 
	//////////////////////////

	////////////////////////
	//Implement Decryption of packets here
	//Call private fucntion to return decrypted form of the data packets received after decompression
	////////////////////////


	////////////////////////
	// Put decrypted and uncompressed data in a queue
	// Make queue accessible by upper level classes which inherit/implement Peer 
	// ,so that they can fetch/peek from queue
	///////////////////////

	///////////////////////
	// If data packet is an ACK
	// then recognize appropriate packet and remove it from the SEND buffer list
	///////////////////////

	std::printf("%s \n",recbuffer);

	Receive();
	return 1;
}

void  Peer::Receive()
{
	DWORD threadId = 0;
	int a = 23;
	CreateThread(NULL,0,(unsigned long(__stdcall *)(void *))runThreadHandler,(void*)this,0,&threadId);
}
