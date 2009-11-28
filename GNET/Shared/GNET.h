
#ifndef _MAP_
#include <map>
#endif

#pragma once

#define PRINT_SOCKADDR(x) printf("Received from %s:%d ==> ", inet_ntoa((x).sin_addr), ntohs((x).sin_port))

namespace GNET {
// ------ GNET_Types.h -------
	#define SA2ULUS( sa ) ulus( (sa).sin_addr.S_un.S_addr, (sa).sin_port)
	#define ULUS2SA( pair, socket ) 	{							\
		socket.sin_family = AF_INET;							\
		socket.sin_addr.S_un.S_addr = pair.first;				\
		socket.sin_port = pair.second; }		
	typedef std::pair<unsigned long, unsigned short> ulus;
	
	///The function in the structure returns true if the data is to be sent to a particular client 
	///and returns false if not
	struct IFilter {
		virtual bool ShouldSend(const ulus &remote) = 0;
	};

	//Structure used by the server for filtering - to send the data to everyone except a particular client
	struct FilterEveryoneBut : IFilter {
	private:
		ulus source;
	public:
		FilterEveryoneBut(SOCKADDR_IN *source) {
			this->source = SA2ULUS(*source);
		}
		bool ShouldSend(const ulus &remote) {
			return (source != remote);
		}
	};

	enum SendFlags
	{
		NONE = 0,
		RELIABLE = 1,
		SEQUENCED = 2,
		AGGREGATED = 4,
		COMPRESSED = 8
	};

// ------ GNET_GamePacket.h -------
	struct IGamePacket {
		virtual ~IGamePacket() {}
		virtual char GetType() = 0;
	};

// ------ GNET_Packet.h -------
	struct PktReg {	
		int size;
		void* (*instantiate) (void);
	};
	typedef std::map<int,GNET::PktReg> PktRegMap;
	typedef std::pair<int,GNET::PktReg> PktRegPair;
	extern PktRegMap g_NetPackets;

	///Macro for - Registration of the Packet 
#define REGISTER_PACKET(T,MAP)	{							\
	GNET::PktReg pkt_data; T t;								\
	pkt_data.size = sizeof(T);								\
	pkt_data.instantiate = t.Instantiate;					\
	MAP.insert(	GNET::PktRegPair(t.GetType(), pkt_data)); }

	///Macro for packet type
#define PACKET_TYPE(NUM,T)								\
	char GetType() { return NUM; }						\
	static void* Instantiate() { return (void*)(new T); }
	
	/// GNET Framework Packets
	#define DATA_PACKET 3

	///Structure with a function which returns the type of the packet.
	struct INetPacket {
		virtual ~INetPacket() {}
		virtual char GetType() = 0;
	};

#pragma pack(push,1)
	struct DataPack : INetPacket
	{
		unsigned int seq_num;
		IGamePacket* game;
		virtual ~DataPack() {
			//delete game;			
		}
		PACKET_TYPE(DATA_PACKET, DataPack);
	};
#pragma pack(pop)



	DataPack* CreateDataPack(IGamePacket* packet);
	extern PktRegMap g_GamePackets;	

// ----- GNET_Peer.h ------
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
		DataPack* Receive(bool should_block, SOCKADDR_IN *sock = 0);

		///Server send function which broadcasts data to all connected clients (according to the Connection Table)
		void Send(INetPacket *pack, IFilter *filter, bool reliable = false);

		///Client send function which creates a DataPacket, adds the sequence number (Reliable UDP) and then 
		///sends the data
		void Send(INetPacket *pack, SOCKADDR_IN *remote, char flag = NONE);

		///Returns the SockAddr of the client who has requested a connection, so that the client can be added 
		///in the connection list
		SOCKADDR_IN* ClientEntered(bool should_block = false);

		///Returns the SockAddr of the client who has disconnected, so that the client can be removed 
		///from the connection list
		SOCKADDR_IN* ClientExited(bool should_block = false);

		///test function to drop random packets for testing the Dead Reckoning - while sending
		void NSimulatorSend(INetPacket *pack, SOCKADDR_IN *remote, bool reliable = false);

		///test function to drop random packets for testing the Dead Reckoning - while receiving
		DataPack* PackLossSimulatorReceive(bool should_block, SOCKADDR_IN *sock = 0, int delay = 0);

		///Receives the data packet from the socket, decodes it and pushes it into the receive buffer
		int recvThread(void);

		///Pops the data from the send buffer, encodes the packet and then sends the data through the socket
		int sendThread(void);

		///Used to log the data that has been received.
		int logcThread(void);

		///Socket Identifier.
		SOCKET socketID;
	};
}