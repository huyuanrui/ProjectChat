#include "common.h"
#include "server.h"

Client *head_online = NULL;
int count = 0;
pthread_mutex_t lock_online;
MYSQL *conn;

void SendUser(Client *pclient)
{
	int i = 0;
	msg rm, sm;
	Client *p = NULL;
	
	sm.type = OTHER;
	printf("how many\n");
	pthread_mutex_lock(&lock_online);
	sprintf(sm.data, "在线用户数量: %d", count);
	if (send(pclient->cfd, &sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	memset(&rm, 0, sizeof(rm));
	if (recv(pclient->cfd, &rm, sizeof(rm), 0) < 0)
	{
		perror("recv error");
		exit(1);
	}
	if (strncmp(rm.data, "YES", 3) == 0)
	{
		if (NULL == head_online)
		{
			printf("head_line error\n");
			exit(1);
		}
		p = head_online->next;
		while (NULL != p)
		{
			i++;
			printf("send %s\n", p->username);
			sprintf(sm.data, "用户%d: %s\n", i, p->username);
			if (send(pclient->cfd, &sm, sizeof(sm), 0) < 0)
			{
				perror("send error");
				exit(1);
			}
			p = p->next;
		}
		printf("end send\n");
	}
	pthread_mutex_unlock(&lock_online);
}

void DelOnline(Client *pclient)
{
	Client *p = NULL, *q = NULL;
	p = FindOnline(pclient->username);
	if (p == NULL)
	{
		printf("FindOline error\n");
		exit(1);
	}
	q = p->next;
	pthread_mutex_lock(&lock_online);
	p->next = q->next;
	free(q);
	count--;
	pthread_mutex_unlock(&lock_online);
}

void CSingleChat(Client *pclient)
{
	Client *receiver = NULL;
	msg rm, sm;
	
	sm.type = OTHER;
	memset(&rm, 0, sizeof(rm));
	if (recv(pclient->cfd, &rm, sizeof(rm), 0) < 0)
	{
		perror("recv error");
		exit(1);
	}
	receiver = FindOnline(rm.data);
	if (NULL == receiver)
	{
		strcpy(sm.data, "对方不在线（输入bye退出）");
		if (send(pclient->cfd, &sm, sizeof(sm), 0) < 0)
		{
			perror("send error");
			exit(1);
		}
	}
	else
	{
		strcpy(sm.data, "我们一起来聊天吧!");
		if (send(pclient->cfd, &sm, sizeof(sm), 0) < 0)
		{
			perror("send error");
			exit(1);
		}
	}
	while (1)
	{
		memset(&rm, 0, sizeof(rm));
		if (recv(pclient->cfd, &rm, sizeof(rm), 0) < 0)
		{
			perror("recv error");
			exit(1);
		}
		if (strncmp(rm.data, "bye", 3) == 0 || pclient->power == 3)
		{
			break;
		}
		if (pclient->power == 2)
		{
			sprintf(sm.data, "您已被群主禁言(输入 bye 退出私聊)");
			if (send(pclient->cfd, &sm, sizeof(sm), 0) < 0)
			{
				perror("send error");
				exit(1);
			}
			continue;
		}
		
		if (NULL == receiver)
		{
			strcpy(sm.data, "对方不在线（输入 bye 退出私聊）");
			if (send(pclient->cfd, &sm, sizeof(sm), 0) < 0)
			{
				perror("send error");
				exit(1);
			}
		}
		else
		{
			sprintf(sm.data, "你收到了来自 %s 的私信: %s", pclient->username, rm.data);
			if (send(receiver->next->cfd, &sm, sizeof(sm), 0) < 0)
			{
				perror("send error");
				exit(1);
			}
		}
	}
	printf("%s end single chat\n", pclient->username);
}

void CGroupChat(Client *pclient)
{
	Client *po = NULL;
	msg rm, sm;
	
	sm.type = OTHER;
	while (1)
	{
		memset(&rm, 0, sizeof(rm));
		if (recv(pclient->cfd, &rm, sizeof(rm), 0) < 0)
		{
			perror("recv error");
			exit(1);
		}
		if (strncmp(rm.data, "bye", 3) == 0 || pclient->power == 3)
		{
			break;
		}
		if (pclient->power == 2)
		{
			sprintf(sm.data, "您已被群主禁言(输入 bye 退出群聊)");
			if (send(pclient->cfd, &sm, sizeof(sm), 0) < 0)
			{
				perror("send error");
				exit(1);
			}
			continue;
		}
		
		sprintf(sm.data, "你收到一个来自 %s 的群消息: %s", pclient->username, rm.data);
		pthread_mutex_lock(&lock_online);
		if (NULL == head_online)
		{
			printf("head_online error\n");
			exit(1);
		}
		po = head_online->next;
		while (po != NULL)
		{
			if (po == pclient)
			{
				po = po->next;
				continue;
			}
			if (send(po->cfd, &sm, sizeof(sm), 0) < 0)
			{
				perror("send error");
				exit(1);
			}
			po = po->next;
		}
		pthread_mutex_unlock(&lock_online);
	}
}

void CSlient(Client *ponline)
{
	Client *pslient = NULL;
	msg sm, rm;
	
	sm.type = OTHER;
	memset(&rm, 0, sizeof(rm));
	if (recv(ponline->cfd, &rm, sizeof(rm.data), 0) < 0)
	{
		perror("recv error");
		exit(1);
	}
	printf("username: %s\n", rm.data);
	pslient = FindOnline(rm.data);
	if (NULL == pslient)
	{
		sprintf(sm.data, "%s用户不存在或已下线\n", rm.data);
		if (send(ponline->cfd, &sm, sizeof(sm), 0) < 0)
		{
			perror("send error");
			exit(1);
		}
		return;
	}
	pslient = pslient->next;	/* 因为找的是父亲结点 */
	if (pslient->power == 0)
	{
		sprintf(sm.data, "无法禁言群主");
		if (send(ponline->cfd, &sm, sizeof(sm), 0) < 0)
		{
			perror("send error");
			exit(1);
		}
		return;
	}
	pslient->power = 2;	/* 2表示禁言 */
	sprintf(sm.data, "禁言%s成功", rm.data);
	if (send(ponline->cfd, &sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	sprintf(sm.data, "您已被群主禁言，不能私聊和群聊，可以接收消息\n");
	if (send(pslient->cfd, (void *)&sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
}

void CLift(Client *ponline)
{
	msg rm, sm;
	Client *plift = NULL;
	
	sm.type= OTHER;
	memset(&rm, 0, sizeof(rm));
	if (recv(ponline->cfd, &rm, sizeof(rm), 0) < 0)
	{
		perror("recv error");
		exit(1);
	}
	printf("username: %s\n", rm.data);
	plift = FindOnline(rm.data);
	if (NULL == plift)
	{
		sprintf(sm.data, "%s不存在或已下线\n", rm.data);
		if (send(ponline->cfd, &sm, sizeof(sm), 0) < 0)
		{
			perror("send error");
			exit(1);
		}
		return;
	}
	plift = plift->next;	/* 因为找的是父亲结点 */
	if (plift->power != 2)
	{
		sprintf(sm.data, "%s用户未被禁言，无法解禁", rm.data);
		if (send(ponline->cfd, &sm, sizeof(sm), 0) < 0)
		{
			perror("send error");
			exit(1);
		}
		return;
	}
	plift->power = 1;	/* 1表示普通用户 */
	sprintf(sm.data, "解除%s的禁言成功", rm.data);
	if (send(ponline->cfd, &sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	sprintf(sm.data, "您已被解禁，可以开始聊天了\n");
	if (send(plift->cfd, (void *)&sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
}

void SRemove(Client *ponline)
{
	Client *premove = NULL;
	msg sm, rm;
	
	sm.type = OTHER;
	memset(&rm, 0, sizeof(rm));
	if (recv(ponline->cfd, &rm, sizeof(rm), 0) < 0)
	{
		perror("recv error");
		exit(1);
	}
	printf("username: %s\n", rm.data);
	premove = FindOnline(rm.data);
	if (NULL == premove)
	{
		sprintf(sm.data, "%s不存在或已下线\n", rm.data);
		if (send(ponline->cfd, &sm, sizeof(sm), 0) < 0)
		{
			perror("send error");
			exit(1);
		}
		return;
	}
	premove = premove->next;	/* 因为找的是父亲结点 */
	if (premove->power == 0)
	{
		sprintf(sm.data, "无法踢出群主");
		if (send(ponline->cfd, &sm, sizeof(sm), 0) < 0)
		{
			perror("send error");
			exit(1);
		}
		return;
	}
	
	premove->power = 3; /* 3表示踢出 */
	sprintf(sm.data, "您已将%s踢出群", rm.data);
	if (send(ponline->cfd, &sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	sprintf(sm.data, "REMOVE");
	if (send(premove->cfd, &sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
}

void send_file(Client *ponline)
{
	msg sm, rm;
	Client *receiver = NULL;
	char filename[50];
	
	sm.type = OTHER;
	memset(&rm, 0, sizeof(rm));
	if (recv(ponline->cfd, &rm, sizeof(rm), 0) < 0)
	{
		perror("recv error");
		exit(1);
	}
	printf("name: %s\n", rm.data);
	receiver = FindOnline(rm.data);
	if (NULL == receiver)
	{
		strcpy(sm.data, "对方不在线（输入bye退出）");
		if (send(ponline->cfd, &sm, sizeof(sm), 0) < 0)
		{
			perror("send error");
			exit(1);
		}
		while (1)
		{
			memset(&rm, 0, sizeof(rm));
			if (recv(ponline->cfd, &rm, sizeof(rm), 0) <= 0)
			{
				perror("recv error");
				exit(1);
			}
			if (0 == strncmp(rm.data, "bye", 3))
			{
				return;
			}
		}
	}
	receiver = receiver->next;
	memset(&rm, 0, sizeof(rm));
	if (recv(ponline->cfd, &rm, sizeof(rm), 0) < 0)
	{
		perror("recv error");
		exit(1);
	}
	printf("filename: %s\n", rm.data);
	
	if (send(receiver->cfd, &rm, sizeof(rm), 0) < 0)	/* 发送文件名 */
	{
		perror("send error");
		exit(1);
	}
	
	while (1)	/* 接收并发送文件内容 */
	{
		memset(&rm, 0, sizeof(rm));
		if (recv(ponline->cfd, &rm, sizeof(rm), 0) < 0)
		{
			perror("recve error");
			exit(1);
		}
		if (rm.type == FILE_END)
		{
			if (send(receiver->cfd, (void *)&rm, sizeof(rm), 0) <= 0)
			{
				perror("send error");
				exit(1);
			}
			sprintf(sm.data, "文件发送成功");
			if (send(ponline->cfd, (void *)&sm, sizeof(sm), 0) < 0)
			{
				perror("send error");
				exit(1);
			}
			break;
		}
		printf("%d", rm.type);
		if (send(receiver->cfd, (void *)&rm, sizeof(rm), 0) <= 0)
		{
			perror("send error");
			exit(1);
		}
	}
}

void SChat(Client *pclient)
{
	int ret;
	msg rm, sm;
	
	sm.type = OTHER;
	while (1)
	{
		memset(&rm, 0, sizeof(rm));
		printf("%s recv chat_type....\n", pclient->username);
		ret = recv(pclient->cfd, &rm, sizeof(rm), 0);	/* 得到操作类型 */
		if (ret < 0)
		{
			perror("recv error");
			exit(1);
		}
		else if (0 == ret)
		{
			printf("The connection with %s is broken!\n", pclient->username);
			close(pclient->cfd);
			DelOnline(pclient);
			pthread_exit(NULL);
		}
		//printf("%s\n", rm.data);
		if (strncmp(rm.data, "HOWMANY", 7) == 0)
		{
			printf("now in senduser\n");
			SendUser(pclient);
		}
		else if (strncmp(rm.data, "SINGLECHAT", 10) == 0)
		{
			printf("now in single_chat\n");
			CSingleChat(pclient);
		}
		else if (strncmp(rm.data, "GROUPCHAT", 9) == 0)
		{
			printf("now in group chat\n");
			CGroupChat(pclient);
		}
		else if (strncmp(rm.data, "CANCEL", 6) == 0)
		{
			printf("%s in cancel\n", pclient->username);
			strncpy(sm.data, "CANCEL", 7);
			if (send(pclient->cfd, &sm, sizeof(sm), 0) < 0)
			{
				perror("send error");
			}
			DelOnline(pclient);
			break;
		}
		else if (strncmp(rm.data, "SLIENT", 6) == 0)
		{
			printf("now in cslient power = %d\n", pclient->power);
			CSlient(pclient);
		}
		else if (strncmp(rm.data, "LIFT", 4) == 0)
		{
			printf("now in lift\n");
			CLift(pclient);
		}
		else if (strncmp(rm.data, "REMOVE", 6) == 0)
		{
			printf("now in remove\n");
			SRemove(pclient);
		}
		else if (strncmp(rm.data, "SENDFILE", 8) == 0)
		{
			printf("now in sendfile\n");
			send_file(pclient);
		}
		else
		{
			printf("%s unknown type\n", pclient->username);
		}
		if (pclient->power == 3)
		{
			printf("%s in cancel\n", pclient->username);
			strncpy(sm.data, "CANCEL", 7);
			if (send(pclient->cfd, &sm, sizeof(sm), 0) < 0)
			{
				perror("send error");
			}
			DelOnline(pclient);
			break;
		}
	}
}

int match_passwd(char *name, char *passwd)
{
	char cmd[100];
	MYSQL_RES *res;
    MYSQL_ROW row;
	sprintf(cmd, "select * from user where name = '%s'", name);
    if (mysql_real_query(conn, cmd, strlen(cmd)))
    {
    	printf("mysql_real_query error\n");
		exit(1);
    }
    res = mysql_store_result(conn);
    row = mysql_fetch_row(res);
    mysql_free_result(res);
    printf("in match name = %s, passwd = %s, power = %s\n", row[0], row[1], row[4]);
    if (0 == strcmp(row[1], passwd))
    {
    	return 1;
    }
    return 0;
}

int is_group_owner(char *name)
{
	char cmd[100];
	MYSQL_RES *res;
    MYSQL_ROW row;
	sprintf(cmd, "select * from user where name = '%s'", name);
    if (mysql_real_query(conn, cmd, strlen(cmd)))
    {
    	printf("mysql_real_query error\n");
		exit(1);
    }
    res = mysql_store_result(conn);
    row = mysql_fetch_row(res);
    mysql_free_result(res);
    if (0 == strncmp(row[4], "0", 1))
    {
    	printf("是群主\n");
    	return 1;
    }
    return 0;
}

void CSignIn(Client *pclient)
{
	Client *p = NULL, *po = NULL, *newpo = NULL;
	char name[50], passwd[50];
	int passwd_count = 0;
	
	printf("now in singin\n");
	memset(name, 0, sizeof(name));
	if (recv(pclient->cfd, name, sizeof(name), 0) < 0)	/* 得到用户名 */
	{
		perror("recv error");
		exit(1);
	}
	printf("username = %s\n", name);
	po = FindOnline(name);
	if (0 == find_user(name))	/* 判断用户是否存在 */
	{
		printf("用户未存在\n");
		if (send(pclient->cfd, "NOTEXIST", 9, 0) < 0)
		{
			perror("send error");
			exit(1);
		}
		return;
	}
	else if (po != NULL)
	{
		printf("用户已在线\n");
		if (send(pclient->cfd, "ONLINE", 7, 0) < 0)
		{
			perror("send error");
			exit(1);
		}
		return;
	}
	else
	{
		strncpy(pclient->username, name, sizeof(pclient->username));
		if (send(pclient->cfd, "OK", 3, 0) < 0)
		{
			perror("send error");
			exit(1);
		}
	}
	while (1)
	{
		memset(passwd, 0, sizeof(passwd));
		if (recv(pclient->cfd, passwd, sizeof(passwd), 0) < 0)	/* 得到用户密码 */
		{
			perror("recv error");
			exit(1);
		}
		printf("passwd: %s\n", passwd);
		if (0 == match_passwd(name, passwd))
		{
			printf("密码错误\n");
			if (send(pclient->cfd, "ERROR", 6, 0) < 0)
			{
				perror("send error");
				exit(1);
			}
			passwd_count++;
			if (passwd_count >= 3)
			{
				return;
			}
			continue;
		}
		else
		{
			if (send(pclient->cfd, "OK", 3, 0) < 0)
			{
				perror("send error");
				exit(1);
			}
			break;
		}
	}
	printf("登录成功\n");
	
	newpo = (Client *)malloc(sizeof(Client));	/* 创建一个新的在线用户 */
	if (NULL == newpo)
	{
		printf("malloc error\n");
		exit(1);
	}
	*newpo = (*pclient);
	if (is_group_owner(pclient->username))
	{
		newpo->power = 0;
	}
	else
	{
		newpo->power = 1;
	}
	sleep(1);
	if (send(pclient->cfd, (void *)&(newpo->power), sizeof(int), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	printf("online: %s %d %d\n", newpo->username, newpo->cfd, newpo->power);
	newpo->next = NULL;
	
	pthread_mutex_lock(&lock_online);	/* 加入在线链表 */
	if (NULL == head_online)
	{
		printf("head_online error\n");
		exit(1);
	}
	po = head_online;
	while (po->next != NULL)
	{
		po = po->next;
	}
	po->next = newpo;
	count++;
	pthread_mutex_unlock(&lock_online);
	
	SChat(newpo);
}

int find_user(char *name)
{
	MYSQL_RES *res;
    MYSQL_ROW row;
    char cmd[50];
    sprintf(cmd, "select * from user where name = '%s'", name);
    if (mysql_real_query(conn, cmd, strlen(cmd)))
    {
    	printf("mysql_real_query error\n");
		exit(1);
    }
    res = mysql_store_result(conn);
    row = mysql_fetch_row(res);
    mysql_free_result(res);
    if (row == NULL)
    {
		return 0;
    }
    return 1;
}

int is_empty_database(void)
{
	MYSQL_RES *res;
    MYSQL_ROW row;
    char cmd[50];
    sprintf(cmd, "select * from user");
    if (mysql_real_query(conn, cmd, strlen(cmd)))
    {
    	printf("mysql_real_query error\n");
		exit(1);
    }
    res = mysql_store_result(conn);
    row = mysql_fetch_row(res);
    mysql_free_result(res);
    if (NULL == row)
    {
    	printf("空数据库\n");
    	return 1;
    }
    return 0;
}

void CSignUp(Client *pclient)
{
	int power = 0;
	char name[100] = {0}, passwd[50] = {0}, cmd[200];
	char question[50] = {0}, answer[50] = {0};
	printf("now in signup\n");
	memset(name, 0, sizeof(name));
	if (recv(pclient->cfd, name, sizeof(name), 0) < 0)	/* 得到用户名 */
	{
		perror("recv error");
		exit(1);
	}
	printf("username = %s\n", name);
	if (0 == find_user(name))	/* 判断用户是否存在 */
	{
		printf("用户未存在\n");
		strncpy(pclient->username, name, sizeof(pclient->username));
		if (send(pclient->cfd, "OK", sizeof("OK"), 0) < 0)
		{
			perror("send error");
			exit(1);
		}
	}
	else
	{
		printf("用户已存在\n");
		if (send(pclient->cfd, "EXISTED", 8, 0) < 0)
		{
			perror("send error");
			exit(1);
		}
		return;
	}
	
	if (recv(pclient->cfd, passwd, sizeof(passwd), 0) < 0)	/* 得到密码 */
	{
		perror("recv error");
		exit(1);
	}
	printf("passwd: %s\n", passwd);
	strncpy(pclient->passwd, passwd, sizeof(pclient->passwd));
	
	if (recv(pclient->cfd, question, sizeof(question), 0) < 0)	/* 得到密保问题 */
	{
		perror("recv error");
		exit(1);
	}	
	if (recv(pclient->cfd, answer, sizeof(answer), 0) < 0)	/* 得到密保问题答案 */
	{
		perror("recv error");
		exit(1);
	}
	printf("question: %s\nanswer: %s\n", question, answer);
	
	if (is_empty_database())
	{
		power = 0;
	}
	else
	{
		power = 1;
	}
	sprintf(cmd, "insert into user(name, passwd, question, answer, power) values('%s', '%s', '%s', '%s', '%d')", name, passwd, question, answer, power);
	printf("cmd = %s\n", cmd);
	if (mysql_real_query(conn, cmd, strlen(cmd)))	/* 插入数据库 */
	{
		printf("%d: mysql_real_query error\n", __LINE__);
		exit(1);
	}
	printf("用户增加成功\n");
}

Client *FindOnline(char *username)
{
	if (NULL == head_online)
	{
		printf("head_online error\n");
		exit(1);
	}
	Client *p = head_online;
	while (p->next != NULL)
	{
		if (strcmp(p->next->username, username) == 0)
		{
			return p;
		}
		p = p->next;
	}
	return NULL;
}

void change_passwd(Client *pclient)
{
	char name[50], answer[50], cmd[100], newpasswd[50];
	MYSQL_RES *res;
    MYSQL_ROW row;
    memset(name, 0, sizeof(name));
	if (recv(pclient->cfd, name, sizeof(name), 0) < 0)
	{
		perror("recv error");
		exit(1);
	}
	printf("name: %s\n", name);
	
	sprintf(cmd, "select * from user where name = '%s'", name);
    if (mysql_real_query(conn, cmd, strlen(cmd)))
    {
    	printf("mysql_real_query error\n");
		exit(1);
    }
    res = mysql_store_result(conn);
    row = mysql_fetch_row(res);
    mysql_free_result(res);
    
	if (0 == row)
	{
		if (send(pclient->cfd, "NOTEXIST", 9, 0) < 0)
		{
			perror("send error");
			exit(1);
		}
		return;
	}
	printf("question: %s\n", row[2]);
	if (send(pclient->cfd, row[2], strlen(row[2]), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	memset(answer, 0, sizeof(answer));
	if (recv(pclient->cfd, answer, sizeof(answer), 0) < 0)
	{
		perror("recv error");
		exit(1);
	}
	printf("answer: %s\n", answer);
	if (0 != strcmp(row[3], answer))
	{
		if (send(pclient->cfd, "NOTMATCH", 9, 0) < 0)
		{
			perror("send error");
			exit(1);
		}
		return;
	}
	else
	{
		if (send(pclient->cfd, "MATCH", 6, 0) < 0)
		{
			perror("send error");
			exit(1);
		}
	}
	memset(newpasswd, 0, sizeof(newpasswd));
	if (recv(pclient->cfd, newpasswd, sizeof(newpasswd), 0) < 0)
	{
		perror("recv error");
		exit(1);
	}
	printf("newpasswd: %s\n", newpasswd);
	sprintf(cmd, "update user set passwd='%s' where name='%s'", newpasswd, name);
	if (mysql_real_query(conn, cmd, strlen(cmd)))
	{
		printf("mysql_real_query error\n");
		exit(1);
	}	
}

void *StartRecv(void *arg)
{
	int ret;
	Client *pclient = (Client *)arg;
	char buf[MAXSIZE] = {0};
	while (1)
	{
		printf("recv......\n");
		memset(buf, 0, sizeof(buf));
		ret = recv(pclient->cfd, buf, sizeof(buf), 0);	/* 得到操作类型 */
		if (ret < 0)
		{
			perror("recv error");
			exit(1);
		}
		else if (0 == ret)
		{
			printf("The connection with %s is broken!\n", pclient->username);
			close(pclient->cfd);
			if (FindOnline(pclient->username) != NULL)
			{
				DelOnline(pclient);
			}
			else
			{
				free(pclient);
			}
			pthread_exit(NULL);
		}
		printf("%s\n", buf);
		if (strncmp(buf, "SIGNUP", 6) == 0)
		{
			CSignUp(pclient);
			printf("end singup\n");
		}
		else if (strncmp(buf, "SIGNIN", 6) == 0)
		{
			CSignIn(pclient);
		}
		else if (strncmp(buf, "CHANGEPD", 8) == 0)
		{
			printf("now in changepasswd\n");
			change_passwd(pclient);
		}
		else if (strncmp(buf, "EXIT", 4) == 0)
		{
			printf("now in client_exit\n");	
			close(pclient->cfd);
			free(pclient);
			pthread_exit(NULL);
		}
		else
		{
			printf("Unknown type\n");
		}
	}
	pthread_exit(NULL);
}

void OnlineInit(void)
{
	head_online = (Client *)malloc(sizeof(Client));
	if (NULL == head_online)
	{
		printf("malloc error\n");
		exit(1);
	}
	head_online->next = NULL;
}

int create_listen_sockfd(void)
{
	int sockfd;
	struct sockaddr_in seraddr;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd)
    {
        perror("socket error");
        exit(1);
    }
    memset(&seraddr, 0, sizeof(seraddr));
    seraddr.sin_family = AF_INET;
    seraddr.sin_port = htons(SPORT);
    seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    if (-1 == bind(sockfd, (struct sockaddr *)&seraddr, sizeof(seraddr)))
    {
        perror("bind error");
        exit(1);
    }
    if (-1 == listen(sockfd, 20000))
    {
        perror("listen error");
        exit(1);
    }
    return sockfd;
}

void connect_mysql()
{
	conn = mysql_init(NULL);
    if (NULL == mysql_real_connect(conn, "localhost", "root", "1", "chat", 0, NULL, 0))
    {
		printf("connect to database error\n");
		exit(1);
    }
    printf("数据库连接成功\n");
}

int main(void)
{
	int connfd, ret, sockfd;  
    struct sockaddr_in clientaddr;
    socklen_t addrlen = sizeof(clientaddr);
    pthread_t thread1;
    Client *pclient = NULL;
    
    sockfd = create_listen_sockfd();
    connect_mysql();
    OnlineInit();
    pthread_mutex_init(&lock_online, NULL);
    while (1)
    {
    	printf("accept......\n");
    	connfd = accept(sockfd, (struct sockaddr *)&clientaddr, &addrlen);
		if (-1 == connfd)
		{
		    perror("accept error");
		    exit(1);
		}
		printf("accept success\n");
		
		pclient = (Client *)malloc(sizeof(Client));
		if (NULL == pclient)
		{
			printf("malloc error\n");
			exit(1);
		}
		memset(pclient, 0, sizeof(Client));
		pclient->cfd = connfd;
		pclient->next = NULL;
		
		printf("create pthread.......\n");
		ret = pthread_create(&thread1, NULL, StartRecv, (void *)pclient);
		if (0 != ret)
		{
		    perror("pthread_create error\n");
		    exit(1);
		}
    }
	return 0;
}
