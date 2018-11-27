#include "common.h"
#include "client.h"

int sockfd, power, flag_exist = 1;
char buf[MAXSIZE];

int ChatMenu(void)
{
	int type;
	if (power == 0)
	{
		while (1)
		{
			printf("\t\t\t————————聊天菜单（群主）————————\n");
			printf("\t\t\t[0]在线人数查询\n");
			printf("\t\t\t[1]私聊\n");
			printf("\t\t\t[2]群聊\n");
			printf("\t\t\t[3]发文件\n");
			printf("\t\t\t[4]注销\n");
			printf("\t\t\t[5]禁言\n");
			printf("\t\t\t[6]解禁\n");
			printf("\t\t\t[7]踢人\n");
			printf("\t\t\t————————————————————————————————\n");
			printf("请输入 0 - 7\n");
			scanf("%d", &type);
			getchar();
			if (type >= 0 && type <= 7)
			{
				break;
			}
			else
			{
				printf("输入错误, 请输入 0-7!\n");
			}
		}
	}
	else
	{
		while (1)
		{
			printf("\t\t\t————————聊天菜单（普通成员）————————\n");
			printf("\t\t\t[0]在线人数查询\n");
			printf("\t\t\t[1]私聊\n");
			printf("\t\t\t[2]群聊\n");
			printf("\t\t\t[3]发文件\n");
			printf("\t\t\t[4]注销\n");
			printf("\t\t\t———————————————————————————————————\n");
			printf("请输入 0 - 4\n");
			scanf("%d", &type);
			setbuf(stdin, NULL);
			if (type >= 0 && type <= 4)
			{
				break;
			}
			else
			{
				printf("输入错误, 请输入 0-4!\n");
			}
		}
	}
	return type;
}

void ShowOnline(void)
{
	int count = 0, i;
	char answer;
	msg sm;
	sm.type = OTHER;
	strncpy(sm.data, "HOWMANY", 8);
	if (send(sockfd, &sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	sleep(1);
	printf("您想查看在线成员列表吗?<y/n>\n");
	scanf(" %c", &answer);
	if (answer == 'y')
	{
		strncpy(sm.data, "YES", 4);
		if (send(sockfd, &sm, sizeof(sm), 0) < 0)
		{
			perror("send error");
			exit(1);
		}
		printf("在线用户列表:\n");
		printf("------------------------\n");
		sleep(2);
		printf("------------------------\n");
	}
}

void SingleChat(void)
{
	msg sm;
	
	sm.type = OTHER;
	strncpy(sm.data, "SINGLECHAT", 11);
	if (send(sockfd, &sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	printf("开始私聊（输入bye退出私聊）\n");
	printf("请输入对方用户名:\n");
	fgets(sm.data, sizeof(sm.data), stdin);
	if (sm.data[strlen(sm.data)-1] == '\n')
	{
		sm.data[strlen(sm.data)-1] = '\0';
	}
	if (send(sockfd, &sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	
	while (1)
	{
		fgets(sm.data, sizeof(sm.data), stdin);
		if (sm.data[strlen(sm.data)-1] == '\n')
		{
			sm.data[strlen(sm.data)-1] = '\0';
		}
		if (send(sockfd, &sm, sizeof(sm), 0) < 0)
		{
			perror("send error");
			exit(1);
		}
		if (strncmp(sm.data, "bye", 3) == 0)
		{
			break;
		}		
	}
}

void GroupChat(void)
{
	msg sm;
	
	sm.type = OTHER;
	strncpy(sm.data, "GROUPCHAT", 10);
	if (send(sockfd, &sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	printf("开始群聊（输入bye退出群聊）\n");
	while (1)
	{
		fgets(sm.data, sizeof(sm.data), stdin);
		if (sm.data[strlen(sm.data)-1] == '\n')	/* 去掉换行符 */
		{
			sm.data[strlen(sm.data)-1] = '\0';
		}
		if (send(sockfd, &sm, sizeof(sm), 0) < 0)
		{
			perror("send error");
			exit(1);
		}
		if (strncmp(sm.data, "bye", 3) == 0)
		{
			printf("end single chat\n");
			break;
		}		
	}
}


void slient(void)
{
	msg sm;
	
	sm.type = OTHER;
	strncpy(sm.data, "SLIENT", 7);
	if (send(sockfd, &sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	printf("您想禁言谁:");
	fgets(sm.data, sizeof(sm.data), stdin);
	if (sm.data[strlen(sm.data)-1] == '\n')
	{
		sm.data[strlen(sm.data)-1] = '\0';
	}
	if (send(sockfd, &sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	sleep(1);
}

void lift(void)
{
	char username[50];
	msg sm;
	
	sm.type= OTHER;
	strncpy(sm.data, "LIFT", 5);
	if (send(sockfd, &sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	printf("您要解除谁的禁言:");
	fgets(sm.data, sizeof(sm.data), stdin);
	if (sm.data[strlen(sm.data)-1] == '\n')
	{
		sm.data[strlen(sm.data)-1] = '\0';
	}
	if (send(sockfd, &sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	sleep(1);
}

void Remove(void)
{
	msg sm;
	
	sm.type = OTHER;
	strncpy(sm.data, "REMOVE", 7);
	if (send(sockfd, &sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	printf("您要踢谁:");
	fgets(sm.data, sizeof(sm.data), stdin);
	if (sm.data[strlen(sm.data)-1] == '\n')
	{
		sm.data[strlen(sm.data)-1] = '\0';
	}
	if (send(sockfd, &sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	sleep(1);
}

void SendFile(void)
{
	msg sm, rm;
	FILE *fp;
	
	sm.type = OTHER;
	strncpy(sm.data, "SENDFILE", 9);
	if (send(sockfd, &sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	printf("你要发文件给谁:\n");
	fgets(sm.data, sizeof(sm.data), stdin);
	if (sm.data[strlen(sm.data)-1] == '\n')
	{
		sm.data[strlen(sm.data)-1] = '\0';
	}
	if (send(sockfd, &sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	
	sm.type = FILENAME;
	printf("请输入文件名:\n");
	fgets(sm.data, sizeof(sm.data), stdin);
	if (sm.data[strlen(sm.data)-1] == '\n')
	{
		sm.data[strlen(sm.data)-1] = '\0';
	}
	if (send(sockfd, &sm, sizeof(sm), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	if (0 == strncmp(sm.data, "bye", 3))
	{
		return;
	}
	
	fp = fopen(sm.data, "r");
	if (NULL == fp)
	{
		printf("fopen error\n");
		exit(1);
	}
	
	sm.type = FILE_CONTENT;
	while (1)
	{
		memset(&sm, 0, sizeof(sm));
		fread(sm.data, sizeof(char), sizeof(sm.data), fp);
		sm.type = FILE_CONTENT;
		//usleep(100);
		if (send(sockfd, (void *)&sm, sizeof(sm), 0) < 0)
		{
			perror("send error");
			exit(1);
		}
		if (ferror(fp))
		{
			printf("ferror error\n");
			break;
		}
		if (feof(fp))
		{
			sm.type = FILE_END;
			fclose(fp);
			if (send(sockfd, (void *)&sm, sizeof(sm), 0) <= 0)
			{
				perror("send error");
				exit(1);
			}
			sleep(1);
			break;
		}
	}
}

void *SendChat(void *arg)
{
	int type, flag = 1;
	msg sm;
	
	sm.type = OTHER;
	/*if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) != 0)
    {
        printf("setpcacelstate error!\n");
        exit(0);
    }
    if (pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) != 0)
    {
        printf("setcanceltype error\n");
        exit(0);
    }*/
	while (flag == 1)
	{
		type = ChatMenu();
		switch(type)
		{
			case 0: ShowOnline();
				break;
			case 1: SingleChat();
				break;
			case 2: GroupChat();
				break;
			case 3: SendFile();
				break;
			case 4: flag = 0;
				strncpy(sm.data, "CANCEL", 7);
				if (send(sockfd, &sm, sizeof(sm), 0) < 0)
				{
					perror("send error\n");
					exit(0);
				}
				break;
			case 5: slient();
				break;
			case 6: lift();
				break;
			case 7: Remove();
				break;
			default:printf("unknown type\n");
				break;
		}
	}
	pthread_exit(NULL);
}

void chat(void)
{
	int ret;
	FILE *fp;
	pthread_t thread_send;
	msg rm;
	char *filename = NULL;
	
	ret = pthread_create(&thread_send, NULL, SendChat, NULL);
	if (0 != ret)
	{
		perror("pthread_create error");
		exit(1);
	}
	while (1)
	{
		memset(&rm, 0, sizeof(rm));
		if (recv(sockfd, (void *)&rm, sizeof(rm), 0) < 0)
		{
			perror("recv error");
			exit(1);
		}
		
		if (rm.type == FILENAME)
		{
			printf("正在接收文件.....\n");
			filename = strrchr(rm.data, '/') + 1;
			if (NULL == filename)
			{
				printf("filename error\n");
			}
			printf("文件名: %s\n", filename);
			fp = fopen(filename, "a");
			if (NULL == fp)
			{
				printf("fopen error\n");
				exit(1);
			}
		}
		else if (rm.type == FILE_END)
		{
			printf("文件接收成功\n");
			fclose(fp);
		}
		else if (rm.type == FILE_CONTENT)
		{
			ret = fwrite(rm.data, sizeof(char), strlen(rm.data), fp);
			if (ferror(fp))
			{
				printf("ferror error\n");
				exit(1);
			}
			if (ret < strlen(rm.data))
			{
				printf("fwrite error\n");
			}
			rm.type = ACK;
			if (send(sockfd, (void *)&rm, sizeof(rm), 0) < 0)
			{
				perror("send error");
				exit(1);
			}
		}
		else if (rm.type == OTHER)
		{		
			if (strncmp(rm.data, "CANCEL", 6) == 0)
			{
				printf("正在注销....\n");
				break;
			}
			else if (strncmp(rm.data, "REMOVE", 6) == 0)
			{
				printf("对不起，我们决定把你踢出这个群\n");
				ret = pthread_cancel(thread_send);
				if (0 != ret)
				{
					perror("pthread_cancel error");
					exit(1);
				}
				pthread_join(thread_send, NULL);
				if (send(sockfd, "CANCEL", 7, 0) < 0)
				{
					perror("send error\n");
					exit(0);
				}
			}
			else
			{
				printf("%s\n", rm.data);
			}
		}
	}
}

void SignIn(void)
{
	int count = 0;
	char username[50], passwd[50], buf[MAXSIZE] = {0};
	printf("\n开始登录......\n");
	if (send(sockfd, "SIGNIN", sizeof("SIGNIN"), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	printf("请输入用户名: ");
	fgets(username, sizeof(username), stdin);
	if (username[strlen(username)-1] == '\n')	/* 去掉换行符 */
	{
		username[strlen(username)-1] = '\0';
	}
	if (send(sockfd, username, strlen(username), 0) < 0)	/* 发送用户名  */
	{
		perror("send error");
		exit(1);
	}
		
	memset(buf, 0, sizeof(buf));
	if (recv(sockfd, buf, sizeof(buf), 0) < 0)	/* 收到服务器回复 */
	{
		perror("recv error");
		exit(1);
	}
	if (strncmp(buf, "NOTEXIST", 8) == 0)	/* 判断用户是否存在 */
	{
		printf("用户不存在，请检查输入是否错误!\n");
		return;
	}
	else if (strncmp(buf, "ONLINE", 6) == 0)	/* 判断用户是否在线 */
	{
		printf("该用户已经在线！\n");
		return;
	}
	while (1)
	{
		printf("请输入密码: ");
		fgets(passwd, sizeof(passwd), stdin);
		if (passwd[strlen(passwd)-1] == '\n')	/* 去掉换行符 */
		{
			passwd[strlen(passwd)-1] = '\0';
		}
		if (send(sockfd, passwd, strlen(passwd), 0) < 0)
		{
			perror("send error");
			exit(1);
		}
		
		memset(buf, 0, sizeof(buf));
		if (recv(sockfd, buf, sizeof(buf), 0) < 0)	/* 收到服务器回复 */
		{
			perror("recv error");
			exit(1);
		}
		if (strncmp(buf, "ERROR", 5) == 0)
		{
			printf("密码错误!(连续3次输错将返回主菜单, 可选择修改密码或退出)\n");
			count++;
			if (count >= 3)
			{
				return;
			}
			continue;
		}
		break;
	}
	printf("登录成功!\n");
	if (recv(sockfd, &power, sizeof(int), 0) < 0)
	{
		perror("recv error");
		exit(1);
	}
	chat();
}

void SingUp(void)
{
	char username[20], passwd[20], passwd2[20], question[50], answer[50];
	printf("\n开始注册......\n");
	if (send(sockfd, "SIGNUP", sizeof("SIGNUP"), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	printf("用户名: ");
	fgets(username, sizeof(username), stdin);
	if (username[strlen(username)-1] == '\n')	/* 去掉换行符 */
	{
		username[strlen(username)-1] = '\0';
	}
	if (send(sockfd, username, strlen(username), 0) < 0)	/* 发送用户名  */
	{
		perror("send error");
		exit(1);
	}

	memset(buf, 0, sizeof(buf));
	if (recv(sockfd, buf, sizeof(buf), 0) < 0)	/* 收到服务器回复 */
	{
		perror("recv error");
		exit(1);
	}
	if (strncmp(buf, "EXISTED", 7) == 0)	/* 判断用户是否存在 */
	{
		printf("用户已存在，请重新设置用户名!\n");
		sleep(1);
		return;
	}
	
	while (1)
	{
		printf("密码: ");
		fgets(passwd, sizeof(passwd), stdin);
		printf("确认密码: ");
		fgets(passwd2, sizeof(passwd), stdin);
		if (strcmp(passwd, passwd2) != 0)
		{
			printf("两次密码不匹配，请重新输入\n\n");
			continue;
		}
		else
		{
			break;
		}		
	}
	if (passwd[strlen(passwd)-1] == '\n')	/* 去掉换行符 */
	{
		passwd[strlen(passwd)-1] = '\0';
	}
	if (send(sockfd, passwd, strlen(passwd), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	
	printf("请输入密保问题:\n");
	fgets(question, sizeof(question), stdin);
	if (question[strlen(question)-1] == '\n')
	{
		question[strlen(question)-1] = '\0';
	}
	if (send(sockfd, question, strlen(question), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	printf("请输入问题答案:\n");
	fgets(answer, sizeof(answer), stdin);
	if (answer[strlen(answer)-1] == '\n')
	{
		answer[strlen(answer)-1] = '\0';
	}
	if (send(sockfd, answer, strlen(answer), 0) < 0)
	{
		perror("send error");
		exit(1);
	}	
	printf("注册成功\n");
}

void ModifyPasswd(void)
{
	char name[50], oldpd[50], newpd[50], newpd2[50];
	char reply[50], answer[50];
	if (send(sockfd, "CHANGEPD", sizeof("CHANGEPD"), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	printf("\n开始修改密码......\n");
	printf("用户名: ");
	fgets(name, sizeof(name), stdin);
	if (name[strlen(name)-1] == '\n')
	{
		name[strlen(name)-1] = '\0';
	}
	if (send(sockfd, name, strlen(name), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	memset(reply, 0, sizeof(reply));
	if (recv(sockfd, reply, sizeof(reply), 0) < 0)
	{
		perror("recv error");
		exit(1);
	}
	if (0 == strncmp(reply, "NOTEXIST", 8))
	{
		printf("用户不存在，请检查是否输入出错\n");
		return;
	}
	printf("请回答密保问题:\n");
	printf("%s:\n", reply);
	fgets(answer, sizeof(answer), stdin);
	if (answer[strlen(answer)-1] == '\n')
	{
		answer[strlen(answer)-1] = '\0';
	}
	if (send(sockfd, answer, strlen(answer), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	memset(reply, 0, sizeof(reply));
	if (recv(sockfd, reply, sizeof(reply), 0) < 0)
	{
		perror("recv error");
		exit(1);
	}
	if (0 == strncmp(reply, "NOTMATCH", 8))
	{
		printf("回答错误\n");
		return;
	}
	printf("回答正确\n");
	while (1)
	{
		printf("新密码: ");
		fgets(newpd, sizeof(newpd), stdin);
		printf("确认新密码: ");
		fgets(newpd2, sizeof(newpd2), stdin);
		if (strcmp(newpd, newpd2) == 0)
		{
			break;
		}
		else
		{
			printf("两次输入的密码不匹配，请重新输入\n\n");
		}
	}
	if (newpd[strlen(newpd)-1] == '\n')
	{
		newpd[strlen(newpd)-1] = '\0';
	}
	if (send(sockfd, newpd, strlen(newpd), 0) < 0)
	{
		perror("send error");
		exit(1);
	}
	printf("密码修改成功\n");
}

int Menu(void)
{
	int type;
	while (1)
	{
		printf("\t\t\t——————————主菜单——————————\n");
		printf("\t\t\t[0]登录\n");
		printf("\t\t\t[1]注册\n");
		printf("\t\t\t[2]修改密码\n");
		printf("\t\t\t[3]退出\n");
		printf("\t\t\t——————————————————————————\n");
		printf("请选择 0 - 3\n");
		scanf("%d", &type);
		setbuf(stdin, NULL);
		if (type >= 0 && type <= 3)
		{
			break;
		}
		else
		{
			printf("输入错误，请选择 0 - 3!\n");
		}
	}
	return type;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
    {
        printf("Please input ip!\n");
        exit(1);
    }
    struct sockaddr_in myaddr;
    int ret, type;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sockfd)
    {
        perror("socket error");
        exit(1);
    }

    memset(&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(SPORT);
    
    ret = inet_pton(AF_INET, argv[1], &myaddr.sin_addr.s_addr);
    if (-1 == ret)
    {
        perror("inet_pton error");
        exit(1);
    }
    else if (0 == ret)
    {
        printf("ip error\n");
        exit(1);
    }
    
    if (-1 == connect(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)))
    {
        perror("connect error");
        exit(0);
    }
    while (1)
    {
    	type = Menu();
    	switch(type)
		{
			case 0: SignIn();
				break;
			case 1: SingUp();
				break;
			case 2: ModifyPasswd();
				break;
			case 3: 
				if (send(sockfd, "EXIT", sizeof("EXIT"), 0) < 0)
				{
					perror("send error");
					exit(1);
				}
				exit(0);
				break;
		}
    }
	return 0;
}
