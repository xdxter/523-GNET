#include<stdio.h>
#include<Winsock2.h>
#include<string.h>
#include <map> 
#include "MySQLDemo.h"
#include "ReadConfig.h"
#include "UDP.h"
#pragma comment (lib,"ws2_32.lib")
#define length 20

using namespace GNET;

MYSQL mysql; // DB connectoin instance

Peer *gnet;

SOCKET socketID;
sockaddr_in serverAddr;
sockaddr &serverAddrCast = (sockaddr&)serverAddr;

char gamehost[128];
int gameport;
char host[128];
int port;

bool isAllReady() {
	bool isReady = true;
	// if (userprepares.size() <= 1) {
	if (false) {
		isReady = false;
	} else {
		pre_map::iterator it;
		for (it = userprepares.begin(); it != userprepares.end(); it++) {
			if (!it->second) {
				isReady = false;
				break;
			}
		}
		printf("isAllReady: true");
	}
	return isReady;
}

void sendAllUserInfoToGameServer() {
	printf("start to send all user info to game server ...");
	UserInfoPack uip;
	int i, size;
	for (i = 0, size = 8; i < size; i++) {
		strcpy(uip.nickname[i], "");
		// sprintf(uip.nickname[i], "Name%d", i);
		uip.position[i] = -1;
		strcpy(uip.ip[i], "");
		// sprintf(uip.ip[i], "IP%d", i);
    }
	printf("UserInfoPack initialized.\n");

	ip_map::iterator it;
	i = 0;
	for (it = ipaddress.begin(); it != ipaddress.end(); it++) {
		getUsernameByUid(uip.nickname[i], it->first);
		uip.position[i] = getPosByUid(it->first);
		strcpy(uip.ip[i], it->second.c_str());
		printf("UserInfo[%d], name[%s], pos[%d], ip[%s]\n", i, uip.nickname[i], uip.position[i], uip.ip[i]);
		i++;
	}

	SOCKADDR_IN clientdst;
	clientdst.sin_family = AF_INET;
	clientdst.sin_port = htons(gameport);
	clientdst.sin_addr.S_un.S_addr = inet_addr(gamehost);
	gnet->Connect(gamehost, gameport);
	gnet->Send(CreateDataPack(&uip), &clientdst, 1);

	printf("sent all user info to game server.");
}

bool saveOrUpdatePrepareStatus(unsigned int uid, bool status) {
	bool isUpdated = false;
	pre_map::iterator it = userprepares.find(uid);
	if (it != userprepares.end()) {
		if (it->second != status) {
			it->second = status;
			isUpdated = true;
			printf("update prepare status: uid[%d], status[%d]\n", uid, status);
		} else {
			isUpdated = false;
			printf("no changes to prepare status: uid[%d], status[%d]\n", uid, status);
		}
	} else {
		userprepares.insert(pre_map::value_type(uid, status));
		printf("add prepare status: uid[%d], status[%d]\n", uid, status);
		isUpdated = true;
	}
	return isUpdated;
}

void removePrepareStatus(unsigned int uid) {
	if (userprepares.size() > 0) {
		pre_map::iterator it = userprepares.find(uid);
		if (it != userprepares.end()) {
			userprepares.erase(it);
			printf("remove user[%d] prepare status.\n", uid);
		}
	}
}

int getPosByUid(unsigned int uid) {
	int pos = -1;
	pos_map::iterator it;
	for (it = positions.begin(); it != positions.end(); it++) {
		if (it->second == uid) {
			pos = it->first;
			break;
		}
    }
	return pos;
}

// to clear a position, remove the position pair value from this map
int getAvailablePosition(int start, int end, unsigned int uid) {
	int pos = -1, i;
	pos_map::iterator it;
	for (i = start; i <= end; i++) {
		it = positions.find(i);
		if (positions.end() != it) {
			// this postion is assigned
			// check if it's the same uid
			if (it->second == uid) {
				pos = it->first;
				break;
			} else {
				continue;
			}
		} else {
			pos = i;
			break;
		}
	}
	return pos;
}

int changeTeam(int oldpos, unsigned int uid) {
	int start, end;
	if (oldpos <= 3) {
		// original in team 1, find position in team 2
		start = 4;
		end = 7;
	} else {
		// original in team 1, find position in team 2
		start = 0;
		end = 4;
	}
	return getAvailablePosition(start, end, uid);
}

int getAvailablePosition(unsigned int uid) {
	return getAvailablePosition(0, 7, uid);
}

void brocast(char *msg, int len, unsigned int excludeUid) {
	ip_map::iterator it;
	for (it = ipaddress.begin(); it != ipaddress.end(); it++) {
		if (it->first != excludeUid) {
			SEND(msg, len, (char *) it->second.c_str(), port);
		}
    }
}

void brocastUSER_LOGIN(char *username, int pos, unsigned int excludeUid) {
	ip_map::iterator it;
	for (it = ipaddress.begin(); it != ipaddress.end(); it++) {
		if (it->first != excludeUid) {
			UserLoginPack m;
			strcpy(m.nickname, username);
			m.position = pos;
			sockaddr_in clientdst;
			clientdst.sin_family = AF_INET;
			clientdst.sin_port = htons(port);
			clientdst.sin_addr.S_un.S_addr = inet_addr(it->second.c_str());
			gnet->Send(CreateDataPack(&m), &clientdst, 1);
		}
    }
}

void sendOutAllUserInfo(char * username) {
	int uid = getUidByUsername(username);
	char * ip = (char *) ipaddress.find(uid)->second.c_str();
	int pos;
	char buf[128];
	char tempname[64];
	ip_map::iterator it;
	for (it = ipaddress.begin(); it != ipaddress.end(); it++) {
		if (it->first != uid) {
			// get this user information and send it to uid
			getUsernameByUid(tempname, it->first);
			pos = getPosByUid(it->first);
			sprintf_s(buf, "6%d%s", pos, tempname);

			UserLoginPack m;
			strcpy(m.nickname, tempname);
			m.position = pos;
			sockaddr_in clientdst;
			clientdst.sin_family = AF_INET;
			clientdst.sin_port = htons(port);
			clientdst.sin_addr.S_un.S_addr = inet_addr(ip);
			gnet->Send(CreateDataPack(&m), &clientdst, 1);
		}
    }
}

void sendOutAllUserInfo(unsigned int uid) {
	char * ip = (char *) ipaddress.find(uid)->second.c_str();
	int pos;
	char username[64];
	char buf[128];
	ip_map::iterator it;
	for (it = ipaddress.begin(); it != ipaddress.end(); it++) {
		if (it->first != uid) {
			// get this user information and send it to uid
			getUsernameByUid(username, uid);
			pos = getPosByUid(uid);
			sprintf_s(buf, "6%d%s", pos, username);
			SEND(buf, sizeof(buf), ip, port);
		}
    }
}

int runThread(void* param)
{
	while(1) {
		int bufferSize = 128;
		char recbuffer[128] = "";

		SOCKADDR_IN remote;
		DataPack *dp = gnet->Receive(true, &remote);
		if (dp) {
			//strcpy(recbuffer, static_cast<MsgPacket*>(dp->game)->msg);
		} else {
			printf("receive error");
		}

		char *clientIP = inet_ntoa(remote.sin_addr);

		printf("recvfrom[%s] from %s:%d\n",recbuffer, clientIP, ntohs(remote.sin_port));

		// TODO: you can handle all the request and response here
		char type = (dp->game)->GetType();
		char buf[256];
		char *msg = recbuffer + 1;
		int uid = -1, pos = -1, oldpos;
		unsigned int excludeUid;
		ChatMsgPack mc;
		AcceptLoginPack la;
		DenyLoginPack ld;
		ChangeStatPack csp;
		UserExitPack eu;
		GameStartPack sg;

		sockaddr_in clientdst;
		ip_map::iterator it;
		switch (type)
		{
			case ACK_OK:
				printf("handle ACK_OK\n");

				sendAllUserInfoToGameServer();

				// got game server ACK, send START to all lobby clients
				strcpy(sg.ip, gamehost);
				sg.port = gameport;
				for (it = ipaddress.begin(); it != ipaddress.end(); it++) {
					clientdst.sin_family = AF_INET;
					clientdst.sin_port = htons(port);
					clientdst.sin_addr.S_un.S_addr = inet_addr((char *) it->second.c_str());
					gnet->Send(CreateDataPack(&sg), &clientdst, 1);
				}
				printf("send START to all lobby clients DONE.\n");
				break;
			case ATTEMPT_LOGIN:
				printf("handle SEND_LOGIN_DATA\n");
				char username[50], password[50];
				
				strcpy(username, static_cast<AttemptLoginPack*>(dp->game)->username);
				strcpy(password, static_cast<AttemptLoginPack*>(dp->game)->password);



				uid = checkLogIn(username, password);
				if (uid != -1) {
					printf("Accept Login\n");
					// save this user information
					ipaddress.insert(ip_map::value_type(uid, clientIP));
					// init prepare status
					userprepares.insert(pre_map::value_type(uid, false));
					// assign a position and send it back
					pos = getAvailablePosition(uid);
					positions.insert(pos_map::value_type(pos, uid));
					sprintf_s(buf, "1%d%s", pos, username);

					strcpy(la.nickname, username);
					la.position = pos;
					clientdst.sin_family = AF_INET;
					clientdst.sin_port = htons(port);
					clientdst.sin_addr.S_un.S_addr = inet_addr(clientIP);
					gnet->Send(CreateDataPack(&la), &clientdst, 1);

					// brocast this user's log in to all other users
					sprintf_s(buf, "6%d%s", pos, username);
					brocastUSER_LOGIN(username, pos, uid);
				} else {
					printf("Deny Login\n");
					clientdst.sin_family = AF_INET;
					clientdst.sin_port = htons(port);
					clientdst.sin_addr.S_un.S_addr = inet_addr(clientIP);
					gnet->Send(CreateDataPack(&ld), &clientdst, 1);
				}
				break;
			case COMFIRM_LOGIN:
				printf("handle ATTEMPT_LOGIN\n");
				// send all existing user information to this user
				strcpy(username, static_cast<ComfirmLoginPack*>(dp->game)->nickname);
				sendOutAllUserInfo(username);
				break;
			case CHAT_MSG:
				printf("handle CHAT_MSG\n");
				
				strcpy(mc.nickname, static_cast<ChatMsgPack*>(dp->game)->nickname);
				strcpy(mc.msg, static_cast<ChatMsgPack*>(dp->game)->msg);

				for (it = ipaddress.begin(); it != ipaddress.end(); it++) {						
					clientdst.sin_family = AF_INET;
					clientdst.sin_port = htons(port);
					clientdst.sin_addr.S_un.S_addr = inet_addr((char *) it->second.c_str());
					gnet->Send(CreateDataPack(&mc), &clientdst, 1);
				}
				break;
			case CHANGE_STAT:
				strcpy(csp.nickname, static_cast<ChangeStatPack*>(dp->game)->nickname);

				switch(static_cast<ChangeStatPack*>(dp->game)->type) {
				case REQUEST_CHANGETEAM:
					printf("handle REQUEST_CHANGETEAM\n");

					oldpos = static_cast<ChangeStatPack*>(dp->game)->position;

					uid = getUidByUsername(csp.nickname);
					pos = changeTeam(oldpos, uid);
					if (pos != -1) {
						// make the changes
						positions.erase(positions.find(oldpos));
						positions.insert(pos_map::value_type(pos, uid));

						csp.ori_position = oldpos;
						csp.position = pos;
						csp.type = REQUEST_CHANGETEAM;
						for (it = ipaddress.begin(); it != ipaddress.end(); it++) {
							clientdst.sin_family = AF_INET;
							clientdst.sin_port = htons(port);
							clientdst.sin_addr.S_un.S_addr = inet_addr((char *) it->second.c_str());
							gnet->Send(CreateDataPack(&csp), &clientdst, 1);
						}

					} else {
						// send back deny message
						printf("Change Team Request Deny!");
					}
					break;
				case REQUEST_QUIT:

					printf("handle REQUEST_QUIT\n");

					uid = getUidByUsername(static_cast<ChangeStatPack*>(dp->game)->nickname);
					pos = getPosByUid(uid);
					if (pos != -1) {
						positions.erase(positions.find(pos));
						ipaddress.erase(ipaddress.find(uid));
						removePrepareStatus(uid);
						printf("user[%d] at [%d] exits.\n", uid, pos);

						strcpy(eu.nickname, static_cast<ChangeStatPack*>(dp->game)->nickname);
						eu.position = pos;
						for (it = ipaddress.begin(); it != ipaddress.end(); it++) {
							if (it->first != uid) {
								clientdst.sin_family = AF_INET;
								clientdst.sin_port = htons(port);
								clientdst.sin_addr.S_un.S_addr = inet_addr((char *) it->second.c_str());
								gnet->Send(CreateDataPack(&eu), &clientdst, 1);
							}
						}
					} else {
						// do nothing
					}
					break;
				case REQUEST_PREPARED:
					printf("handle REQUEST_PREPARED\n");

					uid = getUidByUsername(static_cast<ChangeStatPack*>(dp->game)->nickname);
					pos = getPosByUid(uid);
					if (pos != -1) {
						if (saveOrUpdatePrepareStatus(uid, true)) {
							csp.position = pos;
							csp.type = REQUEST_PREPARED;
							for (it = ipaddress.begin(); it != ipaddress.end(); it++) {
								clientdst.sin_family = AF_INET;
								clientdst.sin_port = htons(port);
								clientdst.sin_addr.S_un.S_addr = inet_addr((char *) it->second.c_str());
								gnet->Send(CreateDataPack(&csp), &clientdst, 1);
							}
							if (isAllReady()) {
								// TODO: send ACK to game server
								printf("send GameAckPack to Game Server\n");
								GameAckPack gap;
								SOCKADDR_IN clientdst;
								clientdst.sin_family = AF_INET;
								clientdst.sin_port = htons(gameport);
								clientdst.sin_addr.S_un.S_addr = inet_addr(gamehost);
								gnet->Send(CreateDataPack(&gap), &clientdst, 1);
								printf("GameAckPack sent.\n");
							} else {
								// do nothing
							}
						} else {
							// no update
						}
					} else {
						// do nothing
					}
					break;
				case REQUEST_CANCEL_PREPARED:
					printf("handle USER_CANCEL_PREPARED\n");
					uid = getUidByUsername(static_cast<ChangeStatPack*>(dp->game)->nickname);
					pos = getPosByUid(uid);
					if (pos != -1) {
						if (saveOrUpdatePrepareStatus(uid, false)) {
							csp.position = pos;
							csp.type = REQUEST_CANCEL_PREPARED;
							for (it = ipaddress.begin(); it != ipaddress.end(); it++) {
								clientdst.sin_family = AF_INET;
								clientdst.sin_port = htons(port);
								clientdst.sin_addr.S_un.S_addr = inet_addr((char *) it->second.c_str());
								gnet->Send(CreateDataPack(&csp), &clientdst, 1);
							}
						} else {
							// no update
						}
					} else {
						// do nothing
					}
					break;
				}
				break;
// 			case UPDATE_GAME:
// 				printf("handle UPDATE_GAME\n");
// 				brocast(recbuffer, sizeof(recbuffer), -1);
//				break;
			default:
				break;
		}
		printf("# of clients: %d\n", ipaddress.size());
		printf("# of positions: %d\n", positions.size());
	}
	return 1;
}
void STARTUP()
{
	WSADATA wsaData;

	WSAStartup(MAKEWORD(2,0), &wsaData);
	socketID = socket(AF_INET,SOCK_DGRAM,0);
}

// create a Thread to handle the recieve so that the traffic is not blocked
void RECEIVE()
{
	DWORD threadId = 0;
	int a = 23;
	CreateThread(NULL,0,(unsigned long(__stdcall *)(void *))runThread,(void *)a,0,&threadId);
}
void LISTEN( int portNo)
{
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(portNo);
	serverAddr.sin_addr.S_un.S_addr = ADDR_ANY;
	
	bind(socketID, &serverAddrCast , sizeof(serverAddrCast));
}
void SEND(char* buf, int len, char *IP , int portNo )
{
	sockaddr_in sendTo;
	sockaddr &sendToCast = (sockaddr&)sendTo;

	sendTo.sin_family = AF_INET;
	sendTo.sin_port = htons(portNo);

	in_addr ipaddr;
	ipaddr.S_un.S_addr = inet_addr(IP);
	struct hostent *hp = gethostbyaddr((char*)&ipaddr,4,AF_INET);
	memcpy((char*)&sendTo.sin_addr,(char*)hp->h_addr_list[0],hp->h_length);

	// MsgPacket m;
	// strcpy(m.msg,buf);
	// gnet->Send(CreateDataPack(&m), &sendTo, 1);

	printf("sent[%s]\n",buf);
}

void main()
{
	REGISTER_PACKET(AttemptLoginPack, GNET::g_GamePackets);
	REGISTER_PACKET(AcceptLoginPack, GNET::g_GamePackets);
	REGISTER_PACKET(DenyLoginPack, GNET::g_GamePackets);
	REGISTER_PACKET(GameStartPack, GNET::g_GamePackets);
	REGISTER_PACKET(ChatMsgPack, GNET::g_GamePackets);
	REGISTER_PACKET(UserLoginPack, GNET::g_GamePackets);
	REGISTER_PACKET(UserExitPack, GNET::g_GamePackets);
	REGISTER_PACKET(ChangeStatPack, GNET::g_GamePackets);
	REGISTER_PACKET(ComfirmLoginPack, GNET::g_GamePackets);

	REGISTER_PACKET(UserInfoPack, GNET::g_GamePackets);
	REGISTER_PACKET(GameAckPack, GNET::g_GamePackets);

	port = getLobbyServerPort();
	getLobbyServerAddress(host);
	printf("lobby: %s port: %d\n", host, port);
	gameport = getGameServerPort();
	getLobbyServerAddress(gamehost);
	printf("game: %s port: %d\n", gamehost, gameport);

	gnet = new Peer();
	gnet->Startup(8,port,50);
	gnet->ListenForConnection(8);

	sendAllUserInfoToGameServer();

	// test send GameAckPack to Game Server
	printf("test send GameAckPack to Game Server\n");
	GameAckPack gap;
	SOCKADDR_IN clientdst;
	clientdst.sin_family = AF_INET;
	clientdst.sin_port = htons(gameport);
	clientdst.sin_addr.S_un.S_addr = inet_addr(gamehost);
	gnet->Send(CreateDataPack(&gap), &clientdst, 1);
	printf("GameAckPack sent.\n");

	initMySQL();
	printAllDBRecord();

	RECEIVE();

	// simulate client requests
	while(1)
	{
		char sendBuf[128];

		// send feekback to client
		gets_s(sendBuf);
		SEND(sendBuf,sizeof(sendBuf),host,port);
	}
	closesocket(socketID);
	mysql_close(&mysql);
}

void initMySQL() {
	mysql_init (&mysql);
	if(!mysql_real_connect(&mysql,MYSQL_HOST,MYSQL_USER,MYSQL_PASSWORD,MYSQL_DATABASE,MYSQL_PORT,NULL,0))
	{ 
		printf("DB connection error!"); 
	}
}

void printAllDBRecord() {
	char *ch_query;
	ch_query="select * from user";
	
	if(mysql_real_query(&mysql,ch_query,(UINT)strlen(ch_query))!=0)
	{ 
		printf("Query Table Error!"); 
	}

	MYSQL_RES *result;
	MYSQL_ROW row;
	
	if(!(result=mysql_use_result(&mysql)))
	{ 
		printf("Read Result Error!"); 		
	}

	printf("ID\tName\tPwd\n");
	while(row=mysql_fetch_row(result)){
		printf("%s\t%s\t%s\n", row[0], row[1], row[2]);
	}

	mysql_free_result(result);
}

int getUidByUsername(char *username) {
	int uid = -1;
	char ch_query[256];
	sprintf_s(ch_query, "select id from user where username='%s'", username);
	if(mysql_real_query(&mysql,ch_query,(UINT)strlen(ch_query))!=0)
	{ 
		printf("Query Table Error!\n"); 
	}
	MYSQL_RES *result;
	MYSQL_ROW row;
	if(!(result=mysql_use_result(&mysql)))
	{ 
		printf("Read Result Error!\n"); 		
	}
	while(row=mysql_fetch_row(result)){
		uid = atoi(row[0]);
	}
	mysql_free_result(result);
	return uid;
}

void getUsernameByUid(char * username, unsigned int uid) {
	char ch_query[256];
	sprintf_s(ch_query, "select username from user where id=%d", uid);
	if(mysql_real_query(&mysql,ch_query,(UINT)strlen(ch_query))!=0)
	{ 
		printf("Query Table Error!\n"); 
	}
	MYSQL_RES *result;
	MYSQL_ROW row;
	if(!(result=mysql_use_result(&mysql)))
	{ 
		printf("Read Result Error!\n"); 		
	}
	while(row=mysql_fetch_row(result)){
		sprintf(username, "%s", row[0]);
	}
	mysql_free_result(result);
}

int checkLogIn(char *username, char *password) {
	int uid = -1;
	char ch_query[256];
	sprintf_s(ch_query, "select id from user where username='%s' and password = '%s'", username, password);
	if(mysql_real_query(&mysql,ch_query,(UINT)strlen(ch_query))!=0)
	{ 
		printf("Query Table Error!\n"); 
	}
	MYSQL_RES *result;
	MYSQL_ROW row;
	if(!(result=mysql_use_result(&mysql)))
	{ 
		printf("Read Result Error!\n"); 		
	}
	while(row=mysql_fetch_row(result)){
		uid = atoi(row[0]);
	}
	mysql_free_result(result);
	return uid;
}