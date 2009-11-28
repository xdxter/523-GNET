#include <map>
#include "GNET_Packet.h"
#include "GNET_GamePacket.h"

using namespace GNET;

PktRegMap GNET::g_GamePackets;


///Creates a Datapacket from the Gamepacket
DataPack* GNET::CreateDataPack(IGamePacket* packet) {
	DataPack* data = new DataPack();
	PktReg *reg = &g_GamePackets[ packet->GetType() ];
	data->game = static_cast<IGamePacket*>(reg->instantiate());
	memcpy(data->game, packet, reg->size );


	return data;
}