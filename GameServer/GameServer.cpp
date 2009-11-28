#include "GameServer.h"
#include "ReadConfig.h"
using namespace GNET;

char gamehost[128];
int gameport;
char lobbyhost[128];
int lobbyport;

void main() {
	REGISTER_PACKET(AttemptLoginPack, GNET::g_GamePackets);
	REGISTER_PACKET(AcceptLoginPack, GNET::g_GamePackets);
	REGISTER_PACKET(DenyLoginPack, GNET::g_GamePackets);
	REGISTER_PACKET(UserInfoPack, GNET::g_GamePackets);
	REGISTER_PACKET(GameAckPack, GNET::g_GamePackets);

	printf("game server start ...\n");

	getLobbyServerAddress(lobbyhost);
	lobbyport = getLobbyServerPort();
	printf("lobby: %s port: %d\n", lobbyhost, lobbyport);
	getGameServerAddress(gamehost);
	gameport = getGameServerPort();
	printf("game: %s port: %d\n", gamehost, gameport);

	Peer *gnet;
	gnet = new Peer();
	gnet->Startup(8,gameport,50);
	gnet->ListenForConnection(8);

	SOCKADDR_IN remote;

	user_ip_map::iterator it;

	while(1) {
		DataPack *dp = gnet->Receive(true, &remote);
		if (dp) {
			char *clientIP = inet_ntoa(remote.sin_addr);
			printf("receive from:%s\n", clientIP);
			
			char type = (dp->game)->GetType();

			GameAckPack gap;
			AcceptLoginPack alp;
			DenyLoginPack dlp;
			SOCKADDR_IN clientdst;
			char temp1[128];
			char temp2[128];
			int pos;

			switch (type) {
				case ATTEMPT_LOGIN:
					printf("handle ATTEMPT_LOGIN\n");
					
					if (isIPValid(clientIP)) {
						gnet->Send(CreateDataPack(&alp), &remote, 1);
						printf("AcceptLoginPack sent.");
					} else {
						gnet->Send(CreateDataPack(&dlp), &remote, 1);
						printf("DenyLoginPack sent.");
					}
					
					break;
				case ACK_OK:
					printf("handle ACK_OK\n");
					// send back ACK to lobby server
					clientdst.sin_family = AF_INET;
					clientdst.sin_port = htons(lobbyport);
					clientdst.sin_addr.S_un.S_addr = inet_addr(lobbyhost);
					gnet->Connect(lobbyhost, lobbyport);
					gnet->Send(CreateDataPack(&gap), &clientdst, 1);
					break;
				case USER_INFO:
					printf("handle USER_INFO\n");
					for(int i = 0, size = 8; i < 8; i++) {
						strcpy(temp1, static_cast<UserInfoPack*>(dp->game)->nickname[i]);
						strcpy(temp2, static_cast<UserInfoPack*>(dp->game)->ip[i]);
						pos = static_cast<UserInfoPack*>(dp->game)->position[i];
						printf("Name[%d]:[%s]\t", i, temp1);
						printf("Pos[%d]:[%d]\t", i, pos);
						printf("IP[%d]:[%s]\n", i, static_cast<UserInfoPack*>(dp->game)->ip[i]);
						// userinfo.insert(user_ip_map::value_type("test", "test"));
						userinfo.insert(user_ip_map::value_type(pos, temp2));
						temp1[0] = 0;
						temp2[0] = 0;
					}

					printAllUserInfo();

					break;
				default:
					printf("unkonw TYPE[%s]\n", type);
					break;
			}

		} else {
			printf("receive error\n");
		}
	}
}

void printAllUserInfo() {
	user_ip_map::iterator it;
	for (it = userinfo.begin(); it != userinfo.end(); it++) {
		printf("Test Output: IP[%s], pos[%d]\n", it->second.c_str(), it->first);
	}
}

bool isIPValid(char * ip2BeTested) {
	bool result = false;
	user_ip_map::iterator it;
	for (it = userinfo.begin(); it != userinfo.end(); it++) {
		if (0 == it->second.compare(ip2BeTested)) {
			result = true;
			break;
		} else {
			continue;
		}
	}
	return result;
}