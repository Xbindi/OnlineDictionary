#include<stdio.h>
#include<string.h>
#include <unistd.h>
#include<stdlib.h>   
#include <arpa/inet.h>
#include <signal.h>
/****************************宏定义*******************************/
#define N 100
#define Q 0 //exit
#define R 1 //user register
#define L 2 //user login
#define W 3 //word
#define H 4 //history
/****************************通信结构体****************************/
typedef struct mystruct
{
	int type;
	char name[N];
	char data[N];
	char word[N];//所要查询的单词
	char time[N];//查询时间
} MSG;
/****************************函数声明******************************/
void do_register(int sockfd);
void do_login(int sockfd);
void next(int sockfd);
void do_word(int sockfd);
void do_history(int sockfd);
/****************************全局变量******************************/
MSG msg;
char buf[N];//接收服务器端回复
/****************************主逻辑*******************************/
int main(int argc, const char *argv[])
{
    char clean[64];
	int sockfd;
	//1.创建套接字，用于连接和通信的
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0)
    {
		perror("socket error");
		exit(-1);
	}

	struct sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);

	//2.connect 连接 
	if(connect(sockfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr)) < 0)
	{
		perror("connect error");
		exit(-1);
	}
	printf("connect server ok ^_^\n");
	while(1)
	{
		printf("\033[33m***********************************************\033[0m\n");
		printf("\033[33m********* 1: regsiter 2: login  0: quit *******\033[0m\n");
		printf("\033[33m***********************************************\033[0m\n");
		printf("please choose : ");

        if(scanf("%d", &msg.type) == 0)
        {
            fgets(clean, 64, stdin);
            printf("\n");
            continue;
        }
		//fgets((char * __restrict__)&msg.type,sizeof(msg.type),stdin);
		//printf("msg.type = %d\n",msg.type);
		switch(msg.type)
		{
		case R: 
			do_register(sockfd);
			break;
		case L:
			do_login(sockfd);
			break;
		// case C:
		// 	system("clear"); 
		// 	break;
		case Q:
			close(sockfd);
			exit(0);
		default:
			printf("unknow cmd please re-enter\n");
			continue;
		}
	}
	close(sockfd);
	return 0;
}
/************************注册**************************************/
//注册函数
void do_register(int sockfd)
{
	printf("input new name : ");
    scanf("%s", msg.name);
	printf("input new password : ");
    scanf("%s", msg.data);
	if (msg.name[0] == '\0'&&msg.data[0]=='\0')
		printf("User name or password cannot be empty\n\n\n");
	while(1)
	{
		if (msg.name[0] != '\0'&&msg.data[0]!='\0')
		{
			break;
		}
        printf("input name cannot be empty: ");
        scanf("%s", msg.name);
        printf("input password cannot be empty: ");
        scanf("%s", msg.data);
        if (msg.name[0] != '\0'&&msg.data[0]!='\0')
		{
			break;
		}
		printf("User name or password cannot be empty\n\n\n");
	}
	send(sockfd,&msg,sizeof(msg),0);

	//接收回复
	recv(sockfd,buf,sizeof(buf),0);
	if (strncmp(buf,"ok",2) == 0)
		printf("register ok ^_^\n");
	else
		printf("user %s already exist register failed >_<\n",msg.name);
}
/************************登录**************************************/
//登录函数
void do_login(int sockfd)
{
	printf("input user name : ");
    scanf("%s", msg.name);
	printf("input password : ");
    scanf("%s", msg.data);
	if (msg.name[0] == '\0'&&msg.data[0]=='\0')
		printf("User name or password cannot be empty\n\n\n");
	while(1)
	{
		if (msg.name[0] != '\0'&&msg.data[0]!='\0')
		{
			break;
		}
        printf("input name cannot be empty: ");
        scanf("%s", msg.name);
        printf("input password cannot be empty: ");
        scanf("%s", msg.data);
	
		if (msg.name[0] != '\0'&&msg.data[0]!='\0')
		{
			break;
		}
		printf("User name or password cannot be empty\n\n\n");
	}
	send(sockfd,&msg,sizeof(msg),0);

	//接收回复
	recv(sockfd,buf,sizeof(buf),0);
	if (strncmp(buf,"ok",2) == 0)
	{
		printf("user %s login ok ^_^\n", msg.name);
		next(sockfd);
	}
	else
		printf("Wrong user name or password >_<\n");
}
/************************二级目录************************************/
void next(int sockfd)
{
	int x=0;
    char clean[64];
	while(1)
	{
		printf("\033[33m***********************************************\033[0m\n");
		printf("\033[33m*** 3: query word 4: hsitory record  0: quit***\033[0m\n");
		printf("\033[33m***********************************************\033[0m\n");
		printf("please choose : ");
        if(scanf("%d", &msg.type) == 0)
        {
            fgets(clean, 64, stdin);
            printf("\n");
            continue;
        }
		switch(msg.type)
		{
		case W: 
			do_word(sockfd);
			break;
		case H:
			do_history(sockfd);
			break;
		// case C:
		// 	system("clear"); 
		// 	break;
		case Q:
			x = 1;
			break;
		}
		if(x==1)//注销
			break;
	}
}
/************************查询单词**************************************/
void do_word(int sockfd)
{	
	while(1)
	{
		printf("Please input word Or Press [#] to exit：");
        scanf("%s", msg.word);
		if((strncmp(msg.word,"#",1))!=0)
		{
			send(sockfd,&msg,sizeof(msg),0);
			recv(sockfd,msg.word,sizeof(msg.word),0);
			printf("Get word ok ：%s\n",msg.word);
		}
		else
			break;
	}
}
/************************查询历史**************************************/
void do_history(int sockfd)
{
	while(1)
	{
		bzero(msg.word,sizeof(msg.word));
		bzero(msg.time,sizeof(msg.time));
        printf("Please enter view history word：");
        scanf("%s", msg.word);
	    send(sockfd,&msg,sizeof(msg),0);
		recv(sockfd,msg.time,sizeof(msg.time),0);
		if (strncmp(msg.time,"quit",4)==0)
		{
			printf("history time get failed >_<!\n");
			break;
		}
		else
		{
			printf("history time get ok ：%s\n",msg.time);
		}		
	}
}
