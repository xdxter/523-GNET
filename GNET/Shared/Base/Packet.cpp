#include <map>
#include <queue>
#include <string.h>
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

void PacketCompressor::compress(char* ptr, int &total_length)
{
	char *p=new char[total_length];
	char begin=*ptr,tab=8;
	int count=1,pcount=0;
	

	for(int i=1;i<total_length;i++)
	{
		if(*(ptr+i)==begin)
		{
			count++;
		}
		else
		{
			if(count>=3)			//compress the same character
			{
				*(p+pcount++)=begin;
				*(p+pcount++)=tab;
				if(begin!=0)
					*(p+pcount++)=count;	//compress int
				begin=*(ptr+i);
				count=1;
			}
			else
			{
				for(int j=0;j<count;j++)
					*(p+pcount++)=begin;
				begin=*(ptr+i);
				count=1;
			}
		}
	}
	*(p+pcount++)='\0';
	memcpy(ptr,p,pcount);
	total_length=pcount;
	delete p;
}

void PacketCompressor::decompress(char* ptr)
{
	int total_length = 100;
	char *p=new char[total_length];
	memcpy(p,ptr,total_length);
	int count=0;
	char begin,tab=8;

	for(int i=0;i<total_length;i++)
	{
		begin=*(p+i);
		if(begin==0)
		{
			if(*(p+i+1)==tab)	//decompress int
			{
				*(ptr+count++)=0;
				*(ptr+count++)=0;
				i++;
			}
			*(ptr+count++)=begin;
		}
		else
		{
			if(*(p+i+1)==tab)	//decompress the same character
			{
				for(int j=0;j<*(p+i+2)-1;j++)
					*(ptr+count++)=begin;
				i+=2;
			}
			*(ptr+count++)=begin;
			
		}
	}
	total_length=count;
	delete p;
}