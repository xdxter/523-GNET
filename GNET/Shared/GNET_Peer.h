#include "GNET_Types.h"
#include "Connections/Connection.h"
#include "Base/Monitor.h"
#include "Base/Turnkey.h"

#pragma once


namespace GNET {

	///This class creates peer for either server/client and takes care of startup,
	///connect,listen for connections,send and receive.
	///NOTE: Packets have to be registered using REGISTER_PACKET, before an object of class peer can be declared.
	class Peer {

	public:
		///Registers all the Packets for the Peer and defines the max packet size to be 1024
		Peer(unsigned int max_packet_size = 1024);

		///Destructor
		virtual ~Peer();

		///Does the WSAData Startup, creates the packet, fills in the address structure, binds the socket
		///and creates threads for send, receive and log.
		int Startup(int max_connections, unsigned short port, int sleep_time = 20);

		///Tries connecting to the server for the maximum number of attempts and inserts it into a List of connections.
		bool Connect(char* ip, unsigned short port, unsigned int max_attempts = 7, unsigned int delay = 500);

		///Listens for connections according to the max number of clients and a timeout value
		int ListenForConnection(int max_clients = 1, unsigned int timeout_ms = 500);

		///Receives data from the receive buffer and returns the packet.
		///@param should_block : If defined, this call will block until a packet is received
		///@param sock : If defined, sock will be filled with the address of the sender
		///@return 0 if no packet has been received, otherwise a reference to the packet.
		DataPack* Receive(bool should_block, SOCKADDR_IN *sock = 0);

		/// Sends a IGamePacket to a list of clients as defined by the IFilter and using any specified flags
		void Peer::Send(IGamePacket *pack, IFilter *filter, char flags);
	
		/// Sends a IGamePacket to an individual client and using any specified flags
		void Peer::Send(IGamePacket *pack, SOCKADDR_IN *remote, char flags);

		/// Sends a INetPacket to a list of clients as defined by the IFilter and using any specified flags
		void Send(INetPacket *pack, IFilter *filter, char flag = NONE);

		/// Sends a INetPacket to an individual client and using any specified flags
		void Send(INetPacket *pack, SOCKADDR_IN *remote, char flag = NONE);

		///Returns the SockAddr of the client who has requested a connection, so that the client can be added 
		///in the connection list
		SOCKADDR_IN* ClientEntered(bool should_block = false);

		///Returns the SockAddr of the client who has disconnected, so that the client can be removed 
		///from the connection list
		SOCKADDR_IN* ClientExited(bool should_block = false);

	protected:	
		/// Wrapper for the thread calling
		static DWORD WINAPI runRecvThread(void* param) { return ((Peer*)param)->recvThread(); }
		/// Wrapper for the thread calling
		static DWORD WINAPI runSendThread(void* param) { return ((Peer*)param)->sendThread(); }
		/// Wrapper for the thread calling
		static DWORD WINAPI runLogcThread(void* param) { return ((Peer*)param)->logcThread(); }

		///Receives the data packet from the socket, decodes it and pushes it into the receive buffer
		int recvThread(void);

		///Pops the data from the send buffer, encodes the packet and then sends the data through the socket
		int sendThread(void);

		///Used to log the data that has been received.
		int logcThread(void);

		///A friend class which manages a specific one-to-one connection
		friend class Connection;

		///A friend class which handles Reliable UDP data transfer.
		friend class ReliableUdp;

		///Connection timeout value before a server decides that a client is no more connected.
		int connect_timeout;

		///Monitor class related to the connection events.
		Monitor<SockBuffer>	connection_events; 

		///Monitor class related to the connection events.
		Monitor<SockBuffer> disconnect_events;

		///Pushes the datagram onto the send buffer
		void Send(Datagram *INetPacket);

	private:
		///Socket Identifier.
		SOCKET socketID;
		
		///Defines the maximum number of connections 
		int max_connections;

		///Defines the maximum number of clients the server can handle
		int max_clients;

		///Defines the sleep time to be used by the Timer class for time based events. 
		int sleep_time;

		///Defines the maximum packet size 
		unsigned int max_packet_size;

		///This is the send buffer which is populated, by the data to be sent.
		Monitor<DgramBuffer> send_buffer;

		///This is the receive buffer populated when data is received on the socket.
		Monitor<DgramBuffer> recv_buffer;
		
		///Used by the game for its own data.
		Monitor<DgramBuffer> game_recv_buffer;

		///This is a connection map - a list of all the current connections.
		ConnectionTable connections;

		HANDLE recv_thread;
		HANDLE send_thread;
	};
}