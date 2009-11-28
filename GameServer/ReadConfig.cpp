#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ReadConfig.h"

void getLobbyServerAddress(char * add) {
	FILE *fp;
	if ((fp = fopen(CONFIG_FILE_NAME, "r")) == NULL) {
		printf("Error occurred while opening the configuration file.\n");
		return;
	}
	char buf[128];
	char key[128];
	char value[128];
	while (!feof(fp)) {
		fgets(buf, 128, fp);
		sscanf(buf, "%s %s\n", key, value);
		if (strcmp(key, "LOBBY_SERVER_ADDRESS") == 0) {
			strcpy(add, value);
			break;
		} else {
			continue;
		}
	}
	fclose(fp);
}

int getLobbyServerPort() {
	FILE *fp;
	if ((fp = fopen(CONFIG_FILE_NAME, "r")) == NULL) {
		printf("Error occurred while opening the configuration file.\n");
		return -1;
	}
	char buf[128];
	char key[128];
	char value[128];
	int port = -1;
	while (!feof(fp)) {
		fgets(buf, 128, fp);
		sscanf(buf, "%s %s\n", key, value);
		if (strcmp(key, "LOBBY_SERVER_PORT") == 0) {
			port = atoi(value);
			break;
		} else {
			continue;
		}
	}
	fclose(fp);
	return port;
}

void getGameServerAddress(char * add) {
	FILE *fp;
	if ((fp = fopen(CONFIG_FILE_NAME, "r")) == NULL) {
		printf("Error occurred while opening the configuration file.\n");
		return;
	}
	char buf[128];
	char key[128];
	char value[128];
	while (!feof(fp)) {
		fgets(buf, 128, fp);
		sscanf(buf, "%s %s\n", key, value);
		if (strcmp(key, "GAME_SERVER_ADDRESS") == 0) {
			strcpy(add, value);
			break;
		} else {
			continue;
		}
	}
	fclose(fp);
}

int getGameServerPort() {
	FILE *fp;
	if ((fp = fopen(CONFIG_FILE_NAME, "r")) == NULL) {
		printf("Error occurred while opening the configuration file.\n");
		return -1;
	}
	char buf[128];
	char key[128];
	char value[128];
	int port = -1;
	while (!feof(fp)) {
		fgets(buf, 128, fp);
		sscanf(buf, "%s %s\n", key, value);
		if (strcmp(key, "GAME_SERVER_PORT") == 0) {
			port = atoi(value);
			break;
		} else {
			continue;
		}
	}
	fclose(fp);
	return port;
}