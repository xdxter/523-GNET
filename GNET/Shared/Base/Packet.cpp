#include <map>
#include <queue>
#include "GNET_Types.h"
#include "GNET_Packet.h"

using namespace GNET; 

PktRegMap GNET::g_NetPackets;
int copyin(char* buff, int offset, char* pack, int size, int parentsize);
int copyout(char* buff, int offset, char* pack, int size, int parentsize);

int PacketEncoder::EncodePacket(INetPacket* pack, char* buffer, int maxsize, int i) {	
	if (dynamic_cast<ConPack*>( pack )) {
		buffer[i++] = (char)CON_PACKET;
		i += copyin(buffer, i, (char*)pack, sizeof(ConPack), sizeof(INetPacket));
		return i;
	}
	if (dynamic_cast<SynPack*>( pack )) {
		buffer[i++] = (char)SYN_PACKET;
		i += copyin(buffer, i, (char*)pack, sizeof(SynPack), sizeof(INetPacket));
		return i;
	}
	if (dynamic_cast<AckPack*>( pack )) {
		buffer[i++] = (char)ACK_PACKET;
		i += copyin(buffer, i, (char*)pack, sizeof(AckPack), sizeof(INetPacket));
		return i;
	}
	if (dynamic_cast<MarcoPack*>( pack )) {
		buffer[i++] = (char)MARCO_PACKET;
		i += copyin(buffer, i, (char*)pack, sizeof(MarcoPack), sizeof(INetPacket));
		return i;
	}
	
	// The complicated one...
	if (dynamic_cast<DataPack*>( pack )) {
		buffer[i++] = (char)DATA_PACKET;
		i += copyin(buffer, i, (char*)pack, sizeof(DataPack) - sizeof(IGamePacket*), sizeof(INetPacket));
		IGamePacket *game = static_cast<DataPack*>(pack)->game;
		//EncodeGamePacket(game, buffer, i, maxsize)
		char type = buffer[i++] = game->GetType();
		i += copyin(buffer, i, (char*)game, g_GamePackets[type].size, sizeof(IGamePacket));
		return i;
	}

	return -1;
}

INetPacket* PacketEncoder::DecodePacket(char* buffer, int i) {
	if (*buffer == CON_PACKET) {
		INetPacket *pack = new ConPack;
		i += copyout(buffer, ++i, (char*)pack, sizeof(ConPack), sizeof(INetPacket));
		return pack;
	}
	if (*buffer == SYN_PACKET) {
		INetPacket *pack = new SynPack;
		i += copyout(buffer, ++i, (char*)pack, sizeof(SynPack), sizeof(INetPacket));
		return pack;
	}
	if (*buffer == ACK_PACKET) {
		INetPacket *pack = new AckPack;
		i += copyout(buffer, ++i, (char*)pack, sizeof(AckPack), sizeof(INetPacket));
		return pack;
	}
	if (*buffer == MARCO_PACKET) {
		INetPacket *pack = new MarcoPack;
		i += copyout(buffer, ++i, (char*)pack, sizeof(MarcoPack), sizeof(INetPacket));
		return pack;
	}

	// The complicated one...
	if (*buffer == DATA_PACKET) {
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

