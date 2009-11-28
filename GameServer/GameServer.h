#if !defined(GameServer_H)

#include<stdio.h>
#include<Winsock2.h>
#include<string.h>
#include <map> 

#include "Packets/GNET_GamePacket.h"
#include "Packets/GNET_Packet.h"
#include "GNET_Peer.h"

#pragma comment (lib,"ws2_32.lib")

#define ATTEMPT_LOGIN			'0' 
#define ACCEPT_LOGIN			'1' 
#define DENY_LOGIN				'2' 

#define USER_INFO				'e'
#define ACK_OK					'o'

int port = 8877;

#pragma pack(push,1)
struct UserInfoPack : GNET::IGamePacket {
	char nickname[10][8];
	int position[8];
	char ip[15][8];
	PACKET_TYPE(USER_INFO, UserInfoPack);
};
#pragma pack(pop)

#pragma pack(push,1)
struct GameAckPack : GNET::IGamePacket {
	PACKET_TYPE(ACK_OK, GameAckPack);
};
#pragma pack(pop)


#pragma pack(push,1)
struct AttemptLoginPack : GNET::IGamePacket {
	char username[10];
	char password[10];
	PACKET_TYPE(ATTEMPT_LOGIN, AttemptLoginPack);
};
#pragma pack(pop)

#pragma pack(push,1)
struct AcceptLoginPack : GNET::IGamePacket {
	char nickname[10];
	int position;
	PACKET_TYPE(ACCEPT_LOGIN, AcceptLoginPack);
};
#pragma pack(pop)

#pragma pack(push,1)
struct DenyLoginPack : GNET::IGamePacket {
	char msg[10];
	PACKET_TYPE(DENY_LOGIN, DenyLoginPack);
};
#pragma pack(pop)

typedef std::map<int, std::string> user_ip_map; // (ip_address, username)
user_ip_map userinfo;

void printAllUserInfo();
bool isIPValid(char * ip2BeTested);

#endif // !defined(GameServer_H)