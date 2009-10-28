#pragma once
#include "winsock2.h"
#include <queue>
#include "GNET_Packet.h"
#include "Connection.h"

namespace GNET {
	struct INetPacket;

	struct Packet
	{
		int systemIndex; /// Server only - this is the index into the player array that this SocketDescriptor maps to
		unsigned int length; /// The length of the data in bytes. Deprecated You should use bitSize.
		unsigned int bitSize; /// The length of the data in bits
		char* data ;/// The data from the sender
		bool deleteData; /// Internal. Indicates whether to delete the data, or to simply delete the packet.
	};

	struct SocketDescriptor
	{
		unsigned long binaryAddress;
		unsigned short port;
	};

	class Peer {
	private:
		SOCKET socketID;
		sockaddr_in sockAddr;

		std::queue<INetPacket> send_buffer;
		std::queue<INetPacket> recv_buffer;

		DWORD WINAPI recvThread(void*);
		DWORD WINAPI sendThread(void*);

	public:
		Peer();
		virtual ~Peer() {}

		int Startup(int connections, float sleep_time,
//		int Startup(int connections, float sleep_time, SocketDescriptor sock_desc, int sock_desc_count);
	//	int Connect( const char* host, unsigned short remote port, const

		int StartUp();
		void ListenOnPort(SocketDescriptor *listenSA);
		SocketDescriptor getSocketDescriptor(char* ip,int port);
		
		// Add a parameter to the SendTo function which allows to choose between reliable/ unreliable UDP
		int SendTo(Packet packet, SocketDescriptor sendToSA);
		void Receive();
		int runThread();

	private:
//		int PackMessage(IPacket pack, int max_size, char* buffer)
	};
}
