#if !defined(MySQLDemo_H)
#define MySQLDemo_H

#include "mysql.h"

#pragma comment(lib,"libmySQL.lib")

// MySQL Connection Information
#define MYSQL_HOST "localhost"
#define MYSQL_USER "root"
#define MYSQL_PASSWORD ""
#define MYSQL_DATABASE "ggenet"
#define MYSQL_PORT 3306

void initMySQL();
void printAllDBRecord();
int getUidByUsername(char *username);
int checkLogIn(char *username, char *password);

#endif // !defined(MySQLDemo_H)