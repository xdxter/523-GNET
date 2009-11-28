#if !defined(UDP_H)

#include "Packets/GNET_GamePacket.h"
#include "Packets/GNET_Packet.h"
#include "GNET_Peer.h"

#define ATTEMPT_LOGIN			'0' 
#define ACCEPT_LOGIN			'1' 
#define DENY_LOGIN				'2' 
#define GAME_START				'3'
#define CHAT_MSG				'4' 
#define USER_LOGIN				'5' 
#define USER_EXIT				'6'
#define CHANGE_STAT				'7'
#define COMFIRM_LOGIN			'8'

#define REQUEST_CHANGETEAM		'a'
#define REQUEST_PREPARED		'b'
#define REQUEST_CANCEL_PREPARED	'c'
#define REQUEST_QUIT			'd'

#define USER_INFO				'e'

#define ACK_OK				'o'

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

#pragma pack(push,1)
struct GameStartPack : GNET::IGamePacket {
	char ip[15];
	int port;
	PACKET_TYPE(GAME_START, GameStartPack);
};
#pragma pack(pop)

#pragma pack(push,1)
struct ChatMsgPack : GNET::IGamePacket {
	char nickname[10];
	char msg[90];
	PACKET_TYPE(CHAT_MSG, ChatMsgPack);
};
#pragma pack(pop)

#pragma pack(push,1)
struct UserLoginPack : GNET::IGamePacket {
	char nickname[10];
	int position;
	PACKET_TYPE(USER_LOGIN, UserLoginPack);
};
#pragma pack(pop)

#pragma pack(push,1)
struct UserExitPack : GNET::IGamePacket {
	char nickname[10];
	int position;
	PACKET_TYPE(USER_EXIT, UserExitPack);
};
#pragma pack(pop)

#pragma pack(push,1)
struct ChangeStatPack : GNET::IGamePacket {
	char type;
	char nickname[10];
	int position;
	int ori_position;
	PACKET_TYPE(CHANGE_STAT, ChangeStatPack);
};
#pragma pack(pop)

#pragma pack(push,1)
struct ComfirmLoginPack : GNET::IGamePacket {
	char nickname[10];
	int position;
	PACKET_TYPE(COMFIRM_LOGIN, ComfirmLoginPack);
};
#pragma pack(pop)

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


void RECEIVE();
void SEND(char* buf, int len, char *IP , int portNo);
void getUsernameByUid(char * username, unsigned int uid);
int getPosByUid(unsigned int uid);

typedef std::map<unsigned int, std::string> ip_map; // (uid, ip_address)
ip_map ipaddress;

typedef std::map<unsigned int, bool> pre_map; // (uid, isPrepared) 
pre_map userprepares;

typedef std::map<unsigned int, unsigned int> pos_map; // (position, uid)
pos_map positions;

#endif // !defined(UDP_H)