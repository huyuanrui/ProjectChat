#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <mysql/mysql.h>
#include <pthread.h>

#define MAXSIZE 1000
#define SPORT 7092
#define FILENAME 0
#define FILE_CONTENT 1
#define FILE_END 2
#define OTHER 3
#define ACK 4

typedef struct msg_
{
	int type;
	char data[MAXSIZE];
}msg;

#endif
