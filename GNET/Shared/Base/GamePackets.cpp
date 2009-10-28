#include <map>
#include "GNET_Packet.h"
#include "GNET_GamePacket.h"

using namespace GNET;

std::map<char, GamePktReg> GNET::g_GamePackets;

DataPack CreateDataPack(IGamePacket* packet) {
	DataPack data;
	GamePktReg *reg = &g_GamePackets[ packet->GetType() ];
	data.game = reg->instantiate();
	memcpy(data.game, packet, reg->size );

	return data;
}