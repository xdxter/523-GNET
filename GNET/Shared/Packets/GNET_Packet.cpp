#include <map>
#include <queue>
#include "GNET_Types.h"
#include "GNET_Packet.h"

using namespace GNET; 

PktRegMap GNET::g_NetPackets;
int copyin(char* buff, int offset, char* pack, int size, int parentsize);
int copyout(char* buff, int offset, char* pack, int size, int parentsize);

int PacketEncoder::EncodePacket(INetPacket* pack, char* buffer, int maxsize, int i) {	
	if (dynamic_cast<DataPack*>( pack )) {
		buffer[i++] = (char)DATA_PACKET;
		i += copyin(buffer, i, (char*)pack, sizeof(DataPack) - sizeof(IGamePacket*), sizeof(INetPacket));
		IGamePacket *game = static_cast<DataPack*>(pack)->game;
		//EncodeGamePacket(game, buffer, i, maxsize)
		char type = buffer[i++] = game->GetType();
		i += copyin(buffer, i, (char*)game, g_GamePackets[type].size, sizeof(IGamePacket));
		return i;
	}

	char type = buffer[i++] = pack->GetType();

	// make sure this packet type actually exists...
	PktRegMap::iterator it;
	if ((it = g_NetPackets.find(type)) == g_NetPackets.end())
		return -1;

	i+= copyin(buffer, i, (char*)pack, it->second.size, sizeof(INetPacket));
	return i;
}

INetPacket* PacketEncoder::DecodePacket(char* buffer, int i) {
	// The complicated one...
	if (buffer[i] == DATA_PACKET) {
		DataPack *pack = new DataPack;
		i += copyout(buffer, ++i, (char*)pack, sizeof(DataPack) - sizeof(IGamePacket*), sizeof(INetPacket));

		int game_packet_type = buffer[i++];
		PktRegMap::iterator it;
		if ((it = g_GamePackets.find(game_packet_type)) != g_GamePackets.end()) {
			PktReg* pkt = &(it->second);
			pack->game = static_cast<IGamePacket*>(pkt->instantiate());
			i += copyout(buffer, i, (char*)(pack->game), pkt->size, sizeof(IGamePacket));
		}
		return pack;
	}

	PktRegMap::iterator it;
	if ((it = g_NetPackets.find(buffer[i])) != g_NetPackets.end()) {
		PktReg *pkt = &(it->second);
		INetPacket *pack = static_cast<INetPacket*>(pkt->instantiate());
		i += copyout(buffer, ++i, (char*)pack, pkt->size, sizeof(INetPacket));
		return pack;
	}

	return 0;
}


// --memcpy function wrappers--
int copyin(char* buff, int offset, char* pack, int size, int psize) {
	size -= psize;
	memcpy(buff+offset, pack + psize, size);
	return size;
}

int copyout(char* buff, int offset, char* pack, int size, int psize) {
	size -= psize;
	memcpy(pack + psize, buff+offset, size);	
	return size;
}

