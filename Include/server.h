#ifndef _SERVER_H
#define _SERVER_H

typedef struct Client_
{
	int cfd;
	//struct sockaddr_in addr;
	char username[50];
	char passwd[50];
	int power;	/* 权限： 0群主、1普通用户、2禁言、3踢出 */
	struct Client_ *next;
}Client;

void SendUser(Client *pclient);
void DelOnline(Client *pclient);
void CSingleChat(Client *pclient);
void CGroupChat(Client *pclient);
void CSlient(Client *ponline);
void CLift(Client *ponline);
void SRemove(Client *ponline);
void send_file(Client *ponline);
void SChat(Client *pclient);
int match_passwd(char *name, char *passwd);
int is_group_owner(char *name);
void CSignIn(Client *pclient);
int find_user(char *name);
int is_empty_database(void);
void CSignUp(Client *pclient);
Client *FindOnline(char *username);
void change_passwd(Client *pclient);
void *StartRecv(void *arg);
void OnlineInit(void);
int create_listen_sockfd(void);
void connect_mysql();

#endif
