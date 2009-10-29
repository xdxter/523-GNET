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

	#define REGISTER_PACKET(T,MAP)	{							\
		GNET::PktReg pkt_data; T t;								\
		pkt_data.size = sizeof(T);								\
		pkt_data.instantiate = t.Instantiate;					\
		MAP.insert(	PktRegPair(t.GetType(), pkt_data)); }

	#define PACKET_TYPE(NUM,T)								\
		char GetType() { return NUM; }						\
		static void* Instantiate() { return (void*)(new T); }
	

	// Framework Packets
	enum {
		CON_PACKET = 0,
		SYN_PACKET,
		ACK_PACKET,
		DATA_PACKET,
		MARCO_PACKET
	};

	struct INetPacket {
		virtual ~INetPacket() {}
		virtual char GetType() = 0;
	};
	
#pragma pack(push, 1)
	struct ConPack : INetPacket
	{
		PACKET_TYPE( CON_PACKET, ConPack )
	};
	struct SynPack : INetPacket
	{
		PACKET_TYPE(SYN_PACKET, SynPack);
	};
	struct AckPack : INetPacket
	{
		PACKET_TYPE(ACK_PACKET, AckPack);
	};

	struct DataPack : INetPacket
	{
		unsigned int seq_num;
		IGamePacket* game;
		virtual ~DataPack() {
			//delete game;			
		}
		PACKET_TYPE(DATA_PACKET, DataPack);
	};
	struct MarcoPack : INetPacket
	{
		unsigned int seq_num;
		PACKET_TYPE(MARCO_PACKET, MarcoPack);
	};
#pragma pack(pop)

	class PacketEncoder {
	public:
		static int EncodePacket(INetPacket* pack, char* buffer, int maxsize, int i = 0);
		static INetPacket* DecodePacket(char* buffer, int i = 0);
		static void RegisterNetPackets() {
			REGISTER_PACKET(ConPack,g_NetPackets);
			REGISTER_PACKET(AckPack,g_NetPackets);
			REGISTER_PACKET(SynPack,g_NetPackets);
			REGISTER_PACKET(DataPack,g_NetPackets);
			REGISTER_PACKET(MarcoPack,g_NetPackets);
		}
	};


	
}