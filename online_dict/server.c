#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>   
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <sqlite3.h>
#include <time.h>
/****************************宏定义*******************************/
#define N 100
#define R 1 //user register
#define L 2 //user login
#define W 3 //query word
#define H 4 //history record

#define DATABASE "dict.db"
#define WOEDTXT "dict.txt"
/****************************通信结构体****************************/
typedef struct mystruct
{
	int type;//指令
	char name[N];//用户名
	char data[N];//密码
	char word[N];//所要查询的单词
	char time[N];//查询时间
} MSG;
/****************************函数声明******************************/
//信号处理函数(处理僵尸进程)
void handler(int signum)
{
	waitpid(-1,NULL,WNOHANG);//非阻塞
	printf("\033[31mrecv %d signal and client exit\033[0m\n", signum);
	//printf("recv %d signal and client exit\n\n\n",signum);
}
void do_register(int acceptfd);//注册
void do_login(int acceptfd);//登录
int do_query(int acceptfd);//查询单词
int sql_register(void);//数据库添加信息函数（注册）
int sql_login(void);//数据库验证信息函数（登录）
int sql_word(void);//数据库验证查找单词函数，查不到去文件查
int history_insert(void);//记录查询时间并写到数据库
int do_history(int acceptfd);//查询数据库中history表并发送给客户端

/****************************全局变量******************************/
MSG msg;
char buf[N];
/****************************主逻辑*******************************/
void do_client(int connfd)
{
    int recvlen = 0;
    
    while(1)
    {
        recvlen = recv(connfd,&msg,sizeof(msg),0);
        if(recvlen < 0)
			printf("recv error !\n");
		else if(recvlen == 0)
			break;
		else
        {
			switch((char)msg.type)
			{
				case R:
					do_register(connfd);//注册
					break;
				case L:
					do_login(connfd);//登录
					break;
				case W:
					do_query(connfd);//查询单词
					break;
				case H:
					do_history(connfd);//查询历史记录
					break;
				default:
					printf("unknown cmd !\n");
					continue;
			}
		}  
    }
	printf("\033[31mclient quit !\033[0m\n");
    if(connfd)
        close(connfd);
	exit(-1);//子进程退出了会发送一个信号，SIGCHLD,需要在前面注册
    return;
}


int main(int argc, const char *argv[])
{
	int sockfd, connfd;
    pid_t pid;

    if(argc < 3)
    {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(-1);
    }    
    //１．创建
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if (sockfd < 0)
	{
		perror("socket error");
		exit(-1);
	}

	//填充结构体
	struct sockaddr_in serveraddr;
    memset(&serveraddr, 0x0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);

	int optval = 1;
	socklen_t optval_len = sizeof(optval);
	// 允许重用本地地址 和端口号   
	setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&optval,optval_len);

	//２．绑定
	if (bind(sockfd,(struct sockaddr *)&serveraddr,sizeof(serveraddr)) < 0)
	{
		perror("bind error");
		exit(-1);
	}

	//３．监听，使sockfd由主动变为被动
	if(listen(sockfd,5) < 0)
	{
		perror("listen error");
		exit(-1);
	}
	printf("\033[31mServer Start !\033[0m\n");

	//注册一个信号 
	signal(SIGCHLD,handler);
    //signal(SIGCHLD, SIG_IGN);

    while(1)
    {
        //４．accept(), 阻塞函数，阻塞等待客户端的连接请求，如果有客户端连接，
		//则accept()函数返回的文件描述符和数返回，返回一个用于通信的套接字文件;
        if((connfd = accept(sockfd, NULL, NULL)) < 0)
        {
            perror("fail to accept");
            exit(-1);
        }
		printf("\033[32mClient Connect OK !\033[0m\n");
        if((pid = fork())< 0)
        {
            perror("fail to fork");
            exit(-1);
        }
        else if(pid == 0)//子进程通信
        {
            if(sockfd)
                close(sockfd);
            do_client(connfd);
        }
        if(connfd)
            close(connfd);
    }

    if(connfd)
        close(connfd);
    if(sockfd)
        close(sockfd);
    return 0;
}
/************************注册**************************************/
//SQL注册
int sql_register(void)
{
	sqlite3 *db; 
	int ret;
	char *errmsg;
	char sql[N] = "\0";
	char buf[N] = "\0";
	ret = sqlite3_open(DATABASE, &db);
	if (ret != SQLITE_OK)   //SQLITR_OK == 0
	{
		printf("register sqlite3_open　error : %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}
	
	sprintf(sql, "create table if not exists User(name text PRIMARY KEY, password text NOT NULL)");
	ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK)
	{
		printf("register sqlite3_exec　error : %s\n", sqlite3_errmsg(db));
		sqlite3_free(errmsg);
		sqlite3_close(db);
		return -1;
	}
	else
	{
		//printf("database create Usr table OK !\n");
	}
    sqlite3_free(errmsg);
	bzero(sql, sizeof(sql));
	sprintf(sql, "insert into User values('%s','%s')", msg.name, msg.data);
	ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK)
	{
		printf("register user failed !\n");
        sqlite3_free(errmsg);
		sqlite3_close(db);
		return -1;
	}
	else
	{
		printf("register user OK !\n");
        sqlite3_free(errmsg);
		sqlite3_close(db);
		return 1;
	}
}

void do_register(int acceptfd)
{
	while(1)
	{
		int result;
		result = sql_register();//调用函数(如果函数返回值为１，发送注册成功，为－１发送注册失败)
		if (result == 1)
		{
			bzero(buf, sizeof(buf));
			strcpy(buf, "ok");
			send(acceptfd, buf, sizeof(buf), 0);
			//printf("send　register ok\n");
			break;
		}
		else if (result == -1)
		{
			bzero(buf, sizeof(buf));
			strcpy(buf, "no");
			send(acceptfd, buf, sizeof(buf), 0);
			//printf("send register　no\n");
			break;
		}
	}
	return;
}
/************************登录**************************************/

//sql登录
int sql_login(void)
{
	sqlite3 *db; 
	int ret;
	char **result;
	int row, column;
	char *errmsg;
	char sql[N] = "\0";
	ret = sqlite3_open(DATABASE, &db);
	if (ret != SQLITE_OK)
	{
		printf("error : %s\n", sqlite3_errmsg(db));
		sqlite3_free(errmsg);
		sqlite3_close(db);
		return -1;
	}
	
	sprintf(sql, "select name,password from User where name='%s';", msg.name);
	ret = sqlite3_get_table(db, sql, &result, &row, &column, &errmsg);
	if (ret != SQLITE_OK)
	{
		printf("login failed !\n");
		sqlite3_free_table(result);
		sqlite3_close(db);
		return -1;
	}
	else
	{
		printf("usr %s login OK !\n", msg.name);
		//result[3]表示name的具体数值
		if (strcmp(result[3], msg.data) == 0)
			ret = 1;
		else
			ret = -1;
	}
	sqlite3_free_table(result);
	sqlite3_close(db);
	return ret;
}

//登录函数
void do_login(int acceptfd)
{
	while(1)
	{
		int result;
		result = sql_login();
		if (result == 1)
		{
			bzero(buf, sizeof(buf));
			strcpy(buf, "ok");
			send(acceptfd, buf, sizeof(buf), 0);
			//printf("send　login ok\n");
			break;
		}
		else if (result == -1)
		{
			bzero(buf, sizeof(buf));
			strcpy(buf, "no");
			send(acceptfd, buf, sizeof(buf), 0);
			//printf("send　login no\n");
			break;
		}
	}
	return;
}

/************************在词库中查询单词释义，并记录客户的查询历史***************************/

int do_query(int acceptfd)
{
	char temp[1024];//用来存放dict文件读取的数据
	FILE *fp;
	char word1[1024];
	int i = 0;
	int result;
	printf("query word : %s\n",msg.word);
	result = sql_word();//调用函数
	if (result == 1)
	{
		send(acceptfd,&msg.word, sizeof(msg.word), 0);
		printf("db found OK !\n");
	}
	else if (result == -1)
	{
		//数据库中没找到，在dict.txt文件找
		if((fp =fopen(WOEDTXT,"r")) == NULL)
		{
			perror("fopen error");
			return -1;
		}

		strcpy(word1, msg.word);
		while (word1[i] != '\0')
		{
			i++;
		}
		word1[i] = ' ';
		word1[i+1] = '\0';

		while (fgets(temp,sizeof(temp), fp) != NULL)
		{
			if (strncmp(temp, word1,i) == 0)
				break;
		}
		history_insert();
		bzero(msg.word, sizeof (msg.word));
		strcpy(msg.word,temp);

		send(acceptfd,msg.word, sizeof(msg.word), 0);
	}
}
//从数据库中查单词
int sql_word(void)
{
	sqlite3 *db; 
	int ret;
	char **result =NULL;
	int row, column;
	char *errmsg;
	char sql[N] = "\0";

	ret = sqlite3_open(DATABASE, &db);
	if (ret != SQLITE_OK)
	{
		printf("sql_word error : %s\n", sqlite3_errmsg(db));
		sqlite3_free(errmsg);
		sqlite3_close(db);
		return -1;
	}
	
	sprintf(sql, "select word,translate from Word where word=\"%s\";", msg.word);
	ret = sqlite3_get_table(db, sql, &result, &row, &column, &errmsg);
	if (ret != SQLITE_OK)
	{
		//printf("sql_word exec failed >_<\n");
		sqlite3_free_table(result);
		sqlite3_close(db);
		return -1;
	}
	else
	{
		//printf("sql_word exec ok ^_^\n");
		if(row!='\0')
		{
			history_insert();
			bzero(msg.word, sizeof(msg.word));
			strcpy(msg.word,result[3]);
			sqlite3_free_table(result);
			sqlite3_close(db);
			return 1;
		}
		else
        {
            //printf("sql word no found >_<\n");
            sqlite3_free_table(result);
			sqlite3_close(db);
			return -1;
        }
	}
}
int history_insert(void)
{
	sqlite3 *db; 
	int ret;
	char timebuf[N];
	char *errmsg;
	char sql[N] = "\0";
	time_t tt;                                           
	struct tm *tm=NULL;

	time(&tt);/* condition */
	tm = localtime(&tt);
	// sprintf(buf,"%d年%d月%d日 %d:%d:%d\n",tm->tm_year+1900,tm->tm_mon+1\
	// 		,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm);

	ret = sqlite3_open(DATABASE, &db);
	if (ret != SQLITE_OK)   //SQLITR_OK == 0
	{
		printf("sqlite3_open　error : %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}

	bzero(sql, sizeof(sql));
	sprintf(sql, "create table if not exists History(name text, word text, time text)");
	ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK)
	{
		printf("sqlite3_exec　error : %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}
	else
	{
		//printf("database create History table ok\n");
	}
    sqlite3_free(errmsg);

	bzero(sql, sizeof(sql));
    sprintf(sql, "insert into History values('%s','%s','%s')",msg.name,msg.word,timebuf);
	ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
	if (ret != SQLITE_OK)
	{
		printf("history insert failed !\n");
        sqlite3_free(errmsg);
		sqlite3_close(db);
		return -1;
	}
	else
	{
		printf("history insert ok !\n");
        sqlite3_free(errmsg);
		sqlite3_close(db);
		return 1;
	}
}
/************************历史查询************************************/
int do_history(int acceptfd)
{
    sqlite3 *db; 
    char *errmsg;
	char **result;
	int row, column;
	int ret;
	char sql[N] = "\0";

    ret = sqlite3_open_v2(DATABASE, &db, SQLITE_OPEN_READONLY | SQLITE_OPEN_NOMUTEX, NULL);
	if (ret != SQLITE_OK)   //SQLITR_OK == 0
	{
		printf("sqlite3_open　error : %s\n", sqlite3_errmsg(db));
		sqlite3_close_v2(db);
		return -1;
	}
    printf("history user:word ===> %s:%s\n",msg.name,msg.word);
    sprintf(sql, "select * from History where word='%s' and name='%s' limit 1;",msg.word,msg.name);//查询哪个单词的历史
	ret = sqlite3_get_table(db, sql, &result, &row, &column, &errmsg);
	if(ret != SQLITE_OK)
	{
		printf("history exec failed !\n");
		sqlite3_free_table(result);
		sqlite3_close_v2(db);
		return -1;		
	}
	else if((ret == SQLITE_OK) && (row == 0))
	{
		printf("history no found !\n");
		bzero(msg.time,sizeof(msg.time));
		strcpy(msg.time,"quit");
		send(acceptfd,msg.time,sizeof(msg.time),0);
		sqlite3_free_table(result);
		sqlite3_close_v2(db);
		return -1;
	}
	else
	{
		printf("history time get OK !\n");		
		bzero(msg.time,sizeof(msg.time));
		strcpy(msg.time,result[column+2]);
		send(acceptfd,msg.time,sizeof(msg.time),0);
 		sqlite3_free_table(result);
		sqlite3_close_v2(db);
		return 0;
	}
}

/*****************************************************************
 * name : history_callback
 * function : 找到一条记录，自动执行一次回调函数
 * input : arg : (调用callback的) 函数传递过来的参数
 *         f_num : 字段数目
 *         f_value : 字段值的指针数组
 *         f_name : 字段名字的指针数组
 * output : success : 0
 *          fail : -1
 * 
 * 注意：回调函数返回0时sqlite3_exec返回值为SQLITE_OK，不是0则是SQLITE_ABORT
 * 同时下次不再调用回调函数。
 * 此处查询仅显示一条结果，故特意成功返回-1表示回调函数被调用，
 * 以此判断sqlite3_exec返回值，是否select语句是否有查询到结果
 * *****************************************************************/
int history_callback(void *arg, int f_num, char **f_value, char **f_name)
{	
	int i;
	int *fd = (int*) arg;

	printf("total column is %d\n",f_num);
 
	for(i = 0;i < f_num; i++)
	{
		printf("col_name　:　%s----> clo_value　:　%s\n",f_name[i],f_value[i]);
	}
	
	strcpy(msg.time,f_value[f_num-1]);
	send(*fd,msg.time,sizeof(msg.time),0);
    return -1;
}

int do_history2(int acceptfd)
{
	sqlite3 *db; 
	int ret;
	char sql[N] = "\0";
	char *errmsg;

	ret = sqlite3_open(DATABASE, &db);
	if (ret != SQLITE_OK)  
	{
		printf("sqlite3_open　error : %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}

	printf("history user:word ===> %s:%s\n",msg.name,msg.word);
	sprintf(sql, "select * from History where word='%s' and name='%s' limit 1;",msg.word,msg.name);//查询哪个单词的历史
	ret = sqlite3_exec(db, sql, history_callback, (void*) &acceptfd, &errmsg);
	if(ret == SQLITE_ABORT)
	{
		printf("history time get OK !\n");
		sqlite3_free(errmsg);
        sqlite3_close(db);
		return 0;	
	}
	else if(ret != SQLITE_OK)
    {
        printf("history sqlite3_exec failed !\n");
        sqlite3_free(errmsg);
        sqlite3_close(db);
        return -1;
    }

	bzero(msg.time,sizeof(msg.time));
	strcpy(msg.time,"quit");
	send(acceptfd,msg.time,sizeof(msg.time),0);
    sqlite3_free(errmsg);
    sqlite3_close(db);
    return 1;
}
