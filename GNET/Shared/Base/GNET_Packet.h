#include <queue>

#pragma once

namespace GNET {
	struct IGamePacket;

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
	};
	
#pragma pack(push, 1)
	struct ConPack : INetPacket
	{
		unsigned int seq_num;
	};
	struct SynPack : INetPacket
	{
		unsigned int seq_num;
	};
	struct AckPack : INetPacket
	{
		unsigned int seq_num;
	};

	struct DataPack : INetPacket
	{
		unsigned int seq_num;
		IGamePacket* game;
		virtual ~DataPack() {
			//delete game;			
		}
	};
	struct MarcoPack : INetPacket
	{
		unsigned int seq_num;
	};
#pragma pack(pop)

	class PacketEncoder {
	public:
		static int EncodePacket(INetPacket* pack, char* buffer, int maxsize, int i = 0);
		static INetPacket* DecodePacket(char* buffer, int i = 0);
	};
	
}