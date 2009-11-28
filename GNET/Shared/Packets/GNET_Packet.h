#include <queue>
#include <map>

#pragma once

namespace GNET {
	struct IGamePacket;

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
		char GetType() const { return NUM; }				\
		static void* Instantiate() { return (void*)(new T); }
	

	/// GNET Framework Packets
	enum {
		CON_PACKET = 0,
		SYN_PACKET,
		ACK_PACKET,
		DATA_PACKET,
		MARCO_PACKET,
		POLO_PACKET,
		RUDP_ACK_PACKET
	};

	///Structure with a function which returns the type of the packet.
	struct INetPacket {
		virtual ~INetPacket() {}
		virtual char GetType() const = 0; 
	};
	
#pragma pack(push, 1)
	///Structure of a Connection Packet
	struct ConPack : INetPacket
	{
		PACKET_TYPE( CON_PACKET, ConPack )
	};

	///Structure of a Synchronization Packet
	struct SynPack : INetPacket
	{
		PACKET_TYPE(SYN_PACKET, SynPack);
	};

	///Structure of a Acknowledge Packet
	struct AckPack : INetPacket
	{		
		PACKET_TYPE(ACK_PACKET, AckPack);
	};

	///Structure of a Data Packet
	struct DataPack : INetPacket
	{
		unsigned int seq_num;
		IGamePacket* game;
		virtual ~DataPack() {
			//delete game;			
		}
		PACKET_TYPE(DATA_PACKET, DataPack);
	};

	///Structure of a Macro Packet
	struct MarcoPack : INetPacket
	{
		PACKET_TYPE(MARCO_PACKET, MarcoPack);
	};

	///Structure of a Polo Packet
	struct PoloPack : INetPacket
	{
		PACKET_TYPE(POLO_PACKET, PoloPack);
	};

	///Structure of a Reliable UDP ACK Packet
	struct RudpAckPack : INetPacket
	{
		int seq_num;
		PACKET_TYPE(RUDP_ACK_PACKET, RudpAckPack);
	};

#pragma pack(pop)

	///Class which contains functions to Encode a packet before sending and decode the packet on receiving.
	class PacketEncoder {
	public:
		///Encodes the packet to be sent.
		static int EncodePacket(INetPacket* pack, char* buffer, int maxsize, int i = 0);

		///Decodes the received packet
		static INetPacket* DecodePacket(char* buffer, int i = 0);

		///Registers the Packets (according to the defined Structure)
		static void RegisterNetPackets() {
			REGISTER_PACKET(ConPack,g_NetPackets);
			REGISTER_PACKET(AckPack,g_NetPackets);
			REGISTER_PACKET(SynPack,g_NetPackets);
			REGISTER_PACKET(DataPack,g_NetPackets);
			REGISTER_PACKET(MarcoPack,g_NetPackets);
			REGISTER_PACKET(PoloPack,g_NetPackets);
			REGISTER_PACKET(RudpAckPack,g_NetPackets);
		}

		/// Creates a deep copy of a subclass of INetPacket.
		static INetPacket* CreateCopy(const INetPacket& pack);
		static IGamePacket* CreateCopy(const IGamePacket& pack);
	};


	
}