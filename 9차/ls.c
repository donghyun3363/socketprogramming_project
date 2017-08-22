#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>		
#include <string.h>
#include <stdlib.h>		
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define BUFFSIZE	1024	// BUFFERSIZE MECRO
#define MEM_SIZE 	1024
#define PORTNO		40051	// MY PORT-NUM MECRO
#define KEY_NUM 	40051

pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
///////////////////////////////////////////////// data struct
struct data				// file data struct
{
	char file_name[100];// file name saving
	int hidden_flag;	// flag saving if it is hidden file 
	char* us_name;	
	char* gr_name;
	mode_t st_mode;
	nlink_t st_nlink;
	off_t st_size;
	blkcnt_t st_blocks;
	
	int wd_flag;		// included in wildcard of directory flag 
	int w_flag;			// wildcard flag
	int d_flag;			// drectory flag
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
};
///////////////////////////////////////////////
void array(struct data* struct_arr, int cnt);	// sort func
int save(struct data* struct_arr, char* G);		// data save into structure func
int del_hidden(struct data* struct_arr, int cnt);// hidden structure delete func
void l_print(struct data* struct_arr, int cnt);	// l option print func
void print(struct data* struct_arr, int cnt);
char* back_strtok(char *str);					// strtok of back func
void w_print(struct data* struct_arr, int cnt, char* al_flag);	// if wildcard, print func
void S_array(struct data* struct_arr, int cnt);	// S option sort func
int ls(int argc, char** argv, char* ser_buf);	// ls func
void sig_handler(int sig);						// signal handler func
int File_cnt;					// file count
char File_temp[100][100];		// file saving 
char* Dir_path;					// path saving
char* wild_path;				// wild card path
int Dirfile_flag;				// file in dir path flag
int wild_flag;					// wild card flag
char* wild_str;					// wildcard string
int hflag;						// h option flag
int sflag;						// s option flag
int lflag;						// l option flag
int Sflag;						// S option flag
int html_flag;					// existing html flag
FILE *file;						// html file
int tf=0;						// -al option flag
////////////////////////////////////////////////// socket programming
struct sockaddr_in server_addr, client_addr; // socket(server, clinet) struct
int socket_fd, client_fd;					// file des variable
char *client_ip;							// client_ip saving variable
//////////////////////////////////////////////////// 
typedef struct Node{			// Node struct
	int pid;
	int status;
	struct Node* pNext;
}NODE;
//////////////////////////////////////////////////// 
int child_make(int socketfd);	// child making func
void child_main(int socketfd);	// child main func
void insert(int Pid);			// 2D insert func
void delete(int Pid);			// 2D delete func
NODE* pHead;					// pHead variable
char** argv;					// argv
int argc=2;						// argc
int len;						// child_addr of len		
int sigint=0;					// SIGINT flag
//////////////////////////////////////////////////// 
time_t ti;						// for time printing
struct tm *lt;
char t_buf[100];
int MaxChild=0;
int MaxSpareServers=0;
int MinSpareServers=0;
int StartServers=5;
int IdleServerCount=0;
int ProcessNumber=5;
void *doit1(void*);
void *doit2(void*);
void *doit3(void*);
void status_switch(int pid, int status);
int main()
{	
	signal(SIGCHLD, sig_handler);	// register SIGCHLD
	signal(SIGINT, sig_handler);	// register SIGINT
	signal(SIGTERM, sig_handler);	// register SIGTERM
	signal(SIGUSR1, sig_handler);	// register SIGUSR1
	signal(SIGUSR2, sig_handler);	// register	SIGUSR2
	int opt;
	int i=0;
	pid_t pid;						// pid struct
	FILE *httpd_fd;					// httpd.conf file des
	char httpd_buf[100]={0,};		
	httpd_fd=fopen("httpd.conf","r");	// file open
	if(httpd_fd<0){						// open error
		printf("Not open httpd.conf file!\n");
		return;
	}
	while(!feof(httpd_fd)){					// from file of end
		memset(httpd_buf, 0, 100);	
		fscanf(httpd_fd, "%s", httpd_buf);	// file read 		
		if(strstr(httpd_buf, "MaxChild")){	// if MaxChild
			fscanf(httpd_fd, "%s", httpd_buf);		
			MaxChild=atoi(httpd_buf);
		}
		else if(strstr(httpd_buf, "MaxSpareServers")){	// if MaxSpareServers
			fscanf(httpd_fd, "%s", httpd_buf);	
			MaxSpareServers=atoi(httpd_buf);		
		}
		else if(strstr(httpd_buf, "MinSpareServers")){	// if MinSpareServers
			fscanf(httpd_fd, "%s", httpd_buf);	
			MinSpareServers=atoi(httpd_buf);		
		}
		else if(strstr(httpd_buf, "StartServers")){	// if StartServers
			fscanf(httpd_fd, "%s", httpd_buf);	
			StartServers=atoi(httpd_buf);		
		}
	}
	fclose(httpd_fd);				// file close
	//////////////////////////////////////////////////// 
	struct hostent *host_entry;		// variable, getting for server ip 
	char host_name[50];
	//////////////////////////////////////////////////// 
	argv=(char**)malloc(sizeof(char*)*4);	// initialize
	for(i=0 ; i<4 ; i++)
		argv[i]=(char*)malloc(sizeof(char)*200);	
	for(i=0 ; i<4 ; i++){
		memset(argv[i], 0, sizeof(char)*200);
	}
	strcpy(argv[0], "ls");
	strcpy(argv[1], "-al");
	//////////////////////////////////////////////////// 
	memset(t_buf, 0, sizeof(char)*100);
	ti=time(NULL);						// getting time
	lt = localtime(&ti);
	strftime(t_buf, 100, "%c", lt);
	//////////////////////////////////////////////////// 
	printf("[%s] Server is started. \n", t_buf);
	if((socket_fd = socket(PF_INET, SOCK_STREAM, 0))<0){// making a socket 
		printf("Server: Can't open stream socket. \n");
		return 0;
	}
	//////////////////////////////////////////////////// setting socket information
	opt=1;
	bzero((char*)&server_addr, sizeof(server_addr));	// socket addr zero
	server_addr.sin_family = AF_INET;					// saving to protocol
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);	// socket server address, big edian
	server_addr.sin_port = htons(PORTNO);				// portnum little edian
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));	
	////////////////////////////////////////////////////
	memset(host_name, 0, sizeof(char)*50);	
	gethostname(host_name, sizeof(char)*50);
	host_entry= gethostbyname(host_name);	// getting struct host_entry
	//////////////////////////////////////////////////// 
	memset(t_buf, 0, sizeof(char)*100);		// getting time
	ti=time(NULL);
	lt = localtime(&ti);
	strftime(t_buf, 100, "%c", lt);
	//////////////////////////////////////////////////// 
    printf( "[%s] Socket is created. Ip: %s, Port: %d\n", t_buf, inet_ntoa( *(struct in_addr*)host_entry->h_addr_list[0]), server_addr.sin_port);
	//////////////////////////////////////////////////// socket binding
	if(bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
		printf("Server: Can't bind local address. \n");	// binding socket with sockaddr_in information
	}
	//////////////////////////////////////////////////// waiting for queue data							
	if(listen(socket_fd, 10)<0){	// listen status
		printf("listen error\n");
	}
	//////////////////////////////////////////////////// 		
	for(i=0;i<StartServers;i++){	// making child process
		IdleServerCount++;			// intitialize idleservercount
		child_make(socket_fd);		// child make func
		sleep(1);
	}		
	for(;;)							// pause
	{
		pause();
	}
	return 0;
}
int child_make(int socketfd)
{
	pid_t pid;
	memset(t_buf, 0, sizeof(char)*100);
//////////////////////////////////////////////////// 
	ti=time(NULL);
	lt = localtime(&ti);
	strftime(t_buf, 100, "%c", lt);
//////////////////////////////////////////////////// 
	pid=fork();			// fork!
	if(pid>0){			// parent part
		insert(pid);	// 2L-D insert
		return 0;
	}
	else if(pid==0){	// child part
		printf("[%s] %d process is created. \n",t_buf ,getpid());		// process creat print
		printf("[%s] IdleServerCount: %d\n",t_buf ,IdleServerCount);	// current ISC print
		child_main(socketfd);	// child main func
	}
}
void child_main(int socketfd)
{
	NODE* pCur=pHead;							// pHead pointer
	char buf[BUFFSIZE];							// read buffer
	char ls_buf[100000];						// html data buffer
	char response[100000];						// response buffer
	DIR *dirp;									// Dir struct
	struct dirent *dir;
	char* temp=NULL;							// tempolary
	char path[200]={0,};						// absolute path 
	int Fflag=0; 								// File flag
	int Dflag=0;								// path flag
	char cwd[200]={0,};							// current working directory
	int flag=0;									// fnmatch flag of ip in file
	pid_t pid;									// pid struct
	FILE *fp=0;									// file fp struct pointer
	char ip_buf[100]={0,};						// saving ip
	int i=0, j=0;
	getcwd(cwd, 1024);							// having cwd
	pthread_t tidB, tidC;						// tidB(doit2), ticC(doit3)
	NODE* pTemp=NULL; 
	///////////////////////////////////////////////////
	memset(buf, 0, BUFFSIZE);					// initializing
	memset(ls_buf, 0, 100000);					
	////////////////////////////////////////////////////
	while(pHead!=NULL){							// remaining of childe delete 2-linkedlist
		pTemp=pHead;
		pHead=pHead->pNext;
		free(pTemp);
	}
	pHead=NULL;
   while(1){
		//////////////////////////////////////////////// initializing
		memset(buf, 0, sizeof(char)*BUFFSIZE);
		memset(ls_buf, 0, sizeof(char)*100000);
		memset(response,0,100000);
   		client_fd=0;
    	len=0;
		for(i=0 ; i<4 ; i++){
			memset(argv[i], 0, sizeof(char)*200);
		}
		strcpy(argv[0], "ls");
		strcpy(argv[1], "-al");
		//////////////////////////////////////////////// accept, waitting for client request
		len = sizeof(client_addr);					// client addr length
		client_fd=accept(socketfd, (struct sockaddr*)&client_addr, &len);
		if(client_fd < 0){							// accept error
			printf("Server: accept faild. \n");
			return;
		}
		pthread_create(&tidB, NULL, &doit2, NULL);	// thread create(to child!, doit2 of connect!)
		kill(getppid(), SIGUSR2);					// signal to parent
		memset(t_buf, 0, sizeof(char)*100);			// getting time
		ti=time(NULL);
		lt = localtime(&ti);
		strftime(t_buf, 100, "%c", lt);		
		////////////////////////////////////////////////
		printf("============ New client ============\n");
		printf("[%s]\n", t_buf);
        client_ip=inet_ntoa(client_addr.sin_addr);// address change through inet_ntoa
		printf("IP: %s\n", client_ip);
		printf("Port: %d\n", client_addr.sin_port);
		printf("PID: %d\n", getpid());
		printf("====================================\n");			
		//////////////////////////////////////////////// access control part
		fp= fopen("accessible.usr","r");			// open accessible.usr of file
		if(fp<0){									// Not open, terminated
			close(client_fd);
			for(i=0 ; i<4 ; i++)					// delete allocation
				free(argv[i]);
			free(argv);
			close(socket_fd);						// socket close
			printf("Noting accessible.usr file!!!\n");
			return;
		}
		while(!feof(fp)){							// from ending
			memset(ip_buf, 0, 100);				
			fscanf(fp, "%s", ip_buf);				// read 1-line in file 
			if(fnmatch(ip_buf, inet_ntoa(client_addr.sin_addr), 0)==0)	// if existing
				flag=1;													// flag set
		}
		fclose(fp);									// close file
		//////////////////////////////////////////////// 
		if(flag==0){		// Not existing ip in file
			// Response Message create
			strcpy(response, "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nConnection: close\r\n");
			strcat(response, "Content-Length: 1000\r\n");
			strcat(response, "Content-Type: text/html\r\n"); 
			strcat(response, "\r\n"); 
			strcat(response, "Access denied!<br>\r");
			strcat(response, "Your Ip: \r");
			strcat(response, inet_ntoa(client_addr.sin_addr));
			strcat(response, "<br>You have no permission to access this web server.<br>\r");
			strcat(response, "HTTP 403.6 - Forbidden: IP address reject<br>\r");
			write(client_fd, response, 100000);		// write response
			close(client_fd);						// close file
			continue;
		}
		tf=1;
		//////////////////////////////////////////////// networking between server and client for transfer data	
		while(read(client_fd, buf, BUFFSIZE) >0){// read
			////////////////////////////////////////////			
			argc=2;								//initializing
			Dflag=0;
			Fflag=0;		
			//////////////////////////////////////////// path decision			
			strtok(buf, " ");
			temp=strtok(NULL, " ");
			if(strstr(temp, "home")==NULL){
				strcpy(path, cwd);
			    strcat(path, temp);
			}
			else
				strcpy(path, temp);
			//////////////////////////////////////////// file or dir decisoin(flag)		
			dirp=opendir(path);			
			if(dirp!=NULL )
			Dflag=1;
			else
				Fflag=1;
			//////////////////////////////////////////// Dflag==1			
			if(Dflag==1){
				if(strcmp(path, "/")){
					if(temp[strlen(path)-1]=='/') // delete end of '/'
						temp[strlen(path)-1]='\0';
						
			    	if(strlen(path)>1){				  // add to argv
						strcpy(argv[2], path);
						argc++;
					}
				}
				ls(argc, argv, ls_buf);				  // ls func call for ls_buf of data
				//////////////////////////////////////// response message
				strcpy(response, "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nConnection: close\r\n");
				strcat(response, "Content-Length: 100000\r\n");
				strcat(response, "Content-Type: text/html\r\n"); // html
				strcat(response, "\r\n"); 
				strcat(response, ls_buf);
				////////////////////////////////////////
			}
			//////////////////////////////////////////// Fflag==1			
			else{
				//////////////////////////////////////// response message
				strcpy(response, "HTTP/1.1 200 OK\r\nAccept-Ranges: bytes\r\nConnection: close\r\n");
				strcat(response, "Content-Length: 100000\r\n");
				strcat(response, "Content-Type: text/plain\r\n"); // general file..
				strcat(response, "\r\n");
				//strcat(response, ls_buf);			  // ls func call for ls_buf of data
				////////////////////////////////////////
			}		
			////////////////////////////////////////////			
			write(client_fd, response, 100000);		// write response message for client
			memset(buf ,0 ,BUFFSIZE);				//ini
			memset(response ,0 ,100000);
		}
		pthread_join(tidB, NULL);					// tidB thread exit
		memset(t_buf, 0, sizeof(char)*100);			// getting time
		ti=time(NULL);
		lt = localtime(&ti);
		strftime(t_buf, 100, "%c", lt);
		////////////////////////////////////////////////
		printf("============ Disconnected client ============\n");	// disconnect write into stdout
		printf("[%s]\n", t_buf);
		client_ip=inet_ntoa(client_addr.sin_addr);
		printf("IP: %s\n", client_ip);
		printf("Port: %d\n", client_addr.sin_port);
		printf("PID: %d\n", getpid());
		printf("=============================================\n");
		close(client_fd);							// client des close!!
		pthread_create(&tidC, NULL, &doit3, NULL);	// thread creat(to child, doit3 of delete)
		kill(getppid(), SIGUSR2);					// siginal to parent
		pthread_join(tidC, NULL);					// tidC thread exit
		////////////////////////////////////////////////	tidB, tidC -> child process thread!	
	}
}
void *doit1(void *vptr)	// tidA, PARENT THREAD
{
	int shm_id;			// shared memory id
	int child_pid=0;	// the child pid
	void *shm_addr;		// shared memory address
	char prev[32];		// save
	char* token=NULL;	
	int i=0, j=0;		// i, j index
	NODE* pCur=pHead;
	NODE* pSave=NULL;
///////////////////////////////////////////////////////////////////////////
	if((shm_id = shmget((key_t)KEY_NUM, MEM_SIZE, IPC_CREAT|0666)) == -1){
		printf("shmget fail\n");				// shared memory create or searching
		return NULL;
	}
	if((shm_addr = shmat( shm_id, (void*)0, 0))==(void*)-1){
		printf("shmat fail\n");					// register
		return NULL;
	}
///////////////////////////////////////////////////////////////////////////
		j=0;
		token=NULL;
		memset((void*)prev, 0, 32);
///////////////////////////////////////////////////////////////////////////
		pthread_mutex_lock(&counter_mutex);
		sleep(1);
		if(strstr((char*)shm_addr, ",1")!=NULL)		// when is status==1(client connect)
		{
			ti=time(NULL);							// getting time
			lt = localtime(&ti);
			strftime(t_buf, 100, "%c", lt);	
			IdleServerCount--;						// Idle decrease
			strcpy(prev, (char*)shm_addr);			// shared memory of values get
			token=strtok(prev, ",");				// child pid get
			child_pid=atoi(token);					
			status_switch(child_pid, 1);			// 2D status set
			j=StartServers-IdleServerCount;		
			if(MaxChild>=ProcessNumber){			// Maxchild condition
				printf("[%s] IdleServerCount: %d\n", t_buf,  IdleServerCount);
				if(MinSpareServers>IdleServerCount){// MinSpareServers condition
					for(i=0 ; i<j ; i++){					
						if(MaxChild==ProcessNumber){	// Maxchild condition
								IdleServerCount++;			// Idle increase
								printf("[%s] Maximum exceed! Not Create child\n", t_buf);
								break;
						}					
						ProcessNumber++;			// process number increase
						IdleServerCount++;			// Idle increase
						kill(getpid(), SIGUSR1);	// process create!
						sleep(1);
					}
				}
			}
		}
		if(strstr((char*)shm_addr, ",0")!=NULL)		// when is status==0(client disconnect)
		{
			ti=time(NULL);							// getting time
			lt = localtime(&ti);
			strftime(t_buf, 100, "%c", lt);
			IdleServerCount++;						// Idle increase
			strcpy(prev, (char*)shm_addr);			// shared memory of values get
			token=strtok(prev, ",");				// child pid get
			child_pid=atoi(token);
			status_switch(child_pid, 0);			// 2D status set
			j=IdleServerCount-StartServers;
			printf("[%s] IdleServerCount: %d\n", t_buf,  IdleServerCount);		
			if(MaxSpareServers<IdleServerCount){	// MaxSpareServers condition			
				for(i=0 ; i<j ; i++){
					pCur=pHead;
					pSave=NULL;
					while(pCur!=NULL){				// searching status=0 of node to last node 
						if(pCur->status==0)
							pSave=pCur;
						pCur=pCur->pNext;
					}							
					kill(pSave->pid, SIGTERM);		// signal pid to sigterm
					sleep(1);
				}	
			}
		}
		pthread_mutex_unlock(&counter_mutex);
		sleep(1);
///////////////////////////////////////////////////////////////////////////
	return NULL;
}
void *doit2(void *vptr) 	// tidB, CHILD THREAD CONNECT
{
	int shm_id;
	void *shm_addr;
	//////////////////////////////////////////////////////////////////////////
	if((shm_id = shmget((key_t)KEY_NUM, MEM_SIZE, IPC_CREAT|0666)) == -1){
		printf("shmget fail\n");				// shared memory create or searching
		return NULL;
	}
	if((shm_addr = shmat( shm_id, (void*)0, 0))==(void*)-1){
		printf("shmat fail\n");					// register
		return NULL;
	}
	///////////////////////////////////////////////////////////////////////////
	pthread_mutex_lock(&counter_mutex);				// mutex!
	sleep(1);
	sprintf((char*)shm_addr, "%d,%d", getpid(), 1);	// write to shared memory (status =1)
	pthread_mutex_unlock(&counter_mutex);
	sleep(1);
		///////////////////////////////////////////////////////////////////////////
	return NULL;
}
void *doit3(void *vptr)	// tidC, CHILD THREAD DISCONNECT
{
	int shm_id;
	void *shm_addr;
	//////////////////////////////////////////////////////////////////////////
	if((shm_id = shmget((key_t)KEY_NUM, MEM_SIZE, IPC_CREAT|0666)) == -1){
		printf("shmget fail\n");				// shared memory create or searching
		return NULL;
	}
	if((shm_addr = shmat( shm_id, (void*)0, 0))==(void*)-1){
		printf("shmat fail\n");					// register
		return NULL;
	}
	///////////////////////////////////////////////////////////////////////////
	pthread_mutex_lock(&counter_mutex);
	sleep(1);
	sprintf((char*)shm_addr, "%d,%d", getpid(), 0); // write to shared memory (status=0)
	pthread_mutex_unlock(&counter_mutex); 
	sleep(1);
	///////////////////////////////////////////////////////////////////////////
	return NULL;
}
void status_switch(int pid, int status)				// switch status in 2D
{
	NODE* pCur=pHead;
	while(pCur!=NULL){
		if(pCur->pid==pid){
			if(status==1)				
				pCur->status=1;		// 1 set
			else
				pCur->status=0;		// 0 set
		}
		pCur=pCur->pNext;
	}
}
////////////////////////////////////////////////// 
void sig_handler(int sig)					
{
	pthread_t tidA;
	NODE* pCur=pHead;
	int cnt=0;
	pid_t pid;					// process struct			
	int i=0;					// idx
	int shm_id;
	if(sig==SIGINT){			// when is SIGINT, Parent part and child ignore!!
		sigint=1;				// flag srt
		while(pCur!=NULL){		// Idle count increase for delete
			if(pCur->status==1){
				status_switch(pCur->pid,0);
				IdleServerCount++;			
			}
			pCur=pCur->pNext;
		}
		pCur=pHead;
		while(pCur!=NULL){		// all of pid information send SIGTERM 
			kill(pCur->pid, SIGTERM);
			pCur=pCur->pNext;			
		}
	}
	else if(sig == SIGTERM){	// when is SIGTERM, Child part!!
		close(client_fd);		// client des close!!
		for(i=0 ; i<4 ; i++)	// delete allocation
			free(argv[i]);
		free(argv);
		exit(0);
	}
	else if(sig == SIGCHLD){	// when is SIGCHLD, Parent part!!
		pid = wait(NULL);		// child clean
		if(sigint==1){			// when is sigint==1
			NODE* pCur=pHead;
			NODE* pTemp=NULL;
			memset(t_buf, 0, sizeof(char)*100);
			ti=time(NULL);
			lt = localtime(&ti);
			strftime(t_buf, 100, "%c", lt);
			while(pHead!=NULL){	// all of 2D node delete
				ProcessNumber--; 
				IdleServerCount--;
				pTemp=pHead;
				pHead=pHead->pNext;
				printf("[%s] %d process is terminated.\n", t_buf,pTemp->pid);
				printf("[%s] IdleServerCount: %d\n", t_buf, IdleServerCount);
				free(pTemp);
			}
			memset(t_buf, 0, sizeof(char)*100);
			ti=time(NULL);			// getting time
			lt = localtime(&ti);
			strftime(t_buf, 100, "%c", lt);
			for(i=0 ; i<4 ; i++)	// delete allocation
				free(argv[i]);
			free(argv);
			close(socket_fd);	// socket close
			printf("[%s] server is terminated.\n", t_buf);
			//////////////////////////////////////////////////////////////////////
			if((shm_id = shmget((key_t)KEY_NUM, MEM_SIZE, IPC_CREAT|0666)) == -1){
				printf("shmget fail\n");
				return;
			}
			if(shmctl(shm_id, IPC_RMID, 0)==-1){
				printf("shmctl fail\n");
			}
			//////////////////////////////////////////////////////////////////////
			exit(0);			// parent process exit
		}
		else{					// sigint==0
			delete(pid);
		}
	}
	else if(sig == SIGUSR1){	// when is SIGUSR1, re fork!! parent part!!		
		child_make(socket_fd);	
	}
	else if(sig == SIGUSR2){	// parent thread create!!!!!
		sleep(1);
		pthread_create(&tidA, NULL, &doit1, NULL);	// tidA
	}
	else{						// another signal
		printf("other signal\n");
		return;
	}
}
void insert(int Pid)
{
	NODE* MyNode;
	NODE* pTemp=pHead;
	MyNode=(NODE*)malloc(sizeof(NODE));	// MyNode allocation
	MyNode->pNext=NULL;
	MyNode->status=0;	
	if(pHead==NULL)				// when it is pHead
			pHead=MyNode;		// pHead set
	else{						// else
			MyNode->pNext=pHead;// pHead re-set
			pHead=MyNode;
	}
	MyNode->pid=Pid;
}
void delete(int Pid)
{
	NODE* pCur=pHead;			// current node pointer
	NODE* pTemp=NULL;			// delete node pointer
	NODE* pPrev=NULL;			// previous node pointer
	if(pHead->pid==Pid){		// it is pHead
		pTemp=pHead;
		pHead=pHead->pNext;		// pHead change
	}
	else{						// else
		pPrev=pCur;		
		while(pCur!=NULL){	
			if(pCur->pid==Pid){	
				pTemp=pCur;
				pPrev->pNext=pCur->pNext;	// Prev node next pointer set
				break;
			}
			pPrev=pCur;
			pCur=pCur->pNext;
		}		
	}
	memset(t_buf, 0, sizeof(char)*100);		// getting time 
	ti=time(NULL);
	lt = localtime(&ti);
	strftime(t_buf, 100, "%c", lt);
	ProcessNumber--;
	IdleServerCount--;
	printf("[%s] %d process is terminated.\n",t_buf ,pTemp->pid);
	printf("[%s] IdleServerCount: %d\n", t_buf, IdleServerCount); 
	free(pTemp);							// delete node
}

////////////////////////////////////////////////// ls function
int ls(int argc, char** argv, char* ser_buf)
{
	DIR *dirp;		// Dir struct
	struct dirent *dir;
	struct data* struct_arr;
	struct stat* buf;	// for stat system call
	char* temp=NULL;	// path saving
	char* token=NULL;	// strtok saving
	int cnt=0;		// dir count
	int i=0;		
	int j=0;
	int total=0;
	int c;			// getopt return value
	int aflag=0;		// option flag
	char cwd[200]={0,};	
	int ls_fd;
	int len_out;

	hflag=0;		// initializing
	sflag=0;
	Sflag=0;
	lflag=0;
	html_flag=0;
	File_cnt=0;		
	Dir_path=NULL;
	Dirfile_flag=0;
	wild_flag=0;
	wild_path=NULL;
	wild_str=NULL;
	//////////////////////////////////////////////////
	getcwd(cwd, 1024);					// current working directory save
	dirp=opendir(".");	
	while((dir=readdir(dirp))!=NULL)
		{
			if(strstr(dir->d_name, ".html")!=NULL)	// .html exception
				html_flag=1;
		}
	closedir(dirp);
	file=fopen("html_ls.html","w");	// html file open
	if(file==0){			// No having file, error 
		printf("error\n");
		fclose(file);
		return 0;
	}
	
	fprintf(file, "<html>\n");	// html start
	fprintf(file, "<title> %s </title>\n", cwd);	// title
	fprintf(file, "<head>\n");	// head
	fprintf(file, "<h2>");		// character size "2"
	for(i=0; i<argc ; i++){		// argv file print
		if(argv[i][0]=='/'){
			if(argv[i][strlen(argv[i])-1]=='/')
				argv[i][strlen(argv[i])-1]='\0';
		}	
		fprintf(file, "<b> %s </b>", argv[i]);
	}
	fprintf(file, "</h2>");	
	fprintf(file, "</head>");
	fprintf(file, "<body>");
	//////////////////////////////////////////////////
	if(tf==1){ aflag=1; lflag=1; }
	while((c=getopt(argc, argv, "alhsS"))!=-1)	// searching the option
	{
		switch(c)
		{ 
			case 'a':		// a option
				aflag=1;
				break;
			case 'l':		// l option
				lflag=1;
				break;
			case 'h':		// h option
				hflag=1;
				break;
			case 's':		// s option
				sflag=1;
				break;
			case 'S':		// S option
				Sflag=1;
				break;
			case '?':		// else option
				printf("No option\n");
				fclose(file);// html file close
				return 0;
			default:		// else
				break;	
		}
	}
	if(argc<2)						// just entering instruction
	{	////////////////////////////////////////////////// current dir reading
		dirp=opendir(cwd);
		while((dir=readdir(dirp))!=NULL)
		{
			if(strstr(dir->d_name, ".html")!=NULL){
				if(html_flag==0){
					continue;
				}
				cnt++;
			}
			else
				cnt++;
		} 
		closedir(dirp);
		//////////////////////////////////////////////////
		struct_arr=(struct data*)malloc(sizeof(struct data)*cnt);// struct array allocation
		memset(struct_arr, 0, sizeof(struct data)*cnt);
		for(j=0 ; j<cnt ; j++)
			memset(struct_arr[j].file_name, 0, sizeof(char)*100);
		//////////////////////////////////////////////////
		if((save(struct_arr, "."))==-1){		// if empty 
			fclose(file);		
			return 0;
		}
		array(struct_arr, cnt);					// struct sorting
		del_hidden(struct_arr,cnt);				// hidden delete
		print(struct_arr, cnt);
		free(struct_arr);						// deallocation
		fclose(file);							// html file close
		return 0;			
	}		
	//////////////////////////////////////////////////
	
	for(i=1 ; i<argc ; i++)
	{ 
		cnt=0;				// initializing
		temp=NULL;
		Dir_path=NULL;
		Dirfile_flag=0;
		token=NULL;
		wild_path=NULL;
		wild_str=NULL;
		wild_flag=0;
		memset(cwd, '0', sizeof(char)*200);			
		getcwd(cwd, 1024);	// current working directory getting

		//////////////////////////////////////////////////
		if(argv[i][0]!='-'||(i == (argc-1)))// No option || Nothing included in option
		{
			
			if(strstr(argv[i], "*")!=NULL || strstr(argv[i], "?") || strstr(argv[i], "[")){	
				wild_flag=1;			// at least, when is existing wildcard
				wild_str=argv[i];
				if(argv[i][0]!='/')		// if path
					temp=cwd;
				else{
					temp=argv[i];
					token=back_strtok(temp);
					wild_str=token;		// wildcard string
					wild_path=temp;		// just path
				}
				
			}			
			else if(argv[i][0]!='/' && argv[i][0]!='.'&&argv[i][0]!='-'){
				strcpy(File_temp[File_cnt], argv[i]);// save file name into 2D array 
				File_cnt++;
				temp=NULL;	
			}
			else if(argv[i][0]=='-'){	// just having option in istr
				temp=cwd;
			}
			else if(strcmp(argv[i],"..")==0){
				temp=cwd;
			}
			else { 						// rest of path in temp
				temp=argv[i];
			}
			////////////////////////////////////////////////////////////////////////////////////////////from now temp decide.
			if(temp == NULL){			// no dir name in instruction
				dirp=opendir(cwd);		// current dir open		
			}
			else{										
				dirp=opendir(temp);		// dir name open
				Dir_path=temp;			// saving the path

				if(dirp==NULL){			// Dir+file case or wild case
					
					token=back_strtok(temp);// token file	
					dirp=opendir(temp);	// token path save to dirp

					if(dirp==NULL){		// Not Dir_file,,, fault direction
						printf("ls: cannot access %s: No such file or directory\n", temp);
						continue;
					}
					else{
						Dirfile_flag=1;	
						strcpy(File_temp[File_cnt], token);	// save file name into 2D array
						File_cnt++;
					} 
				}
			}
			if(dirp == NULL && token==NULL){// when is'0', error
				printf("ls: cannot access %s: No such file or directory\n", temp);
				continue;
			}
			while((dir=readdir(dirp))!=NULL)// dir count
			{
				if(strstr(dir->d_name, ".html")!=NULL){
					if(html_flag==0){
						continue;
					}
					cnt++;
				}
				else
					cnt++;
			}
			closedir(dirp);
			struct_arr=(struct data*)malloc(sizeof(struct data)*cnt);	// struct array allcation 
			memset(struct_arr, 0, sizeof(struct data)*cnt);
			for(j=0 ; j<cnt ; j++)
			memset(struct_arr[j].file_name, 0, sizeof(char)*100);	
			////////////////////////////////////////////////////////////////////////////////////////////
			if(aflag==0 && lflag==0)// nothing -a, and -lDir_flag
			{	
				if((save(struct_arr, temp))==-1){// no having file or dir
					fprintf(file, "</body>\n");	// html body close
					fprintf(file, "</html>\n");	// html close
					fclose(file);				// html file close
					return 0;
				} 
				if(File_cnt != 0)				// cnt set into File_cnt 
					cnt = File_cnt;
				array(struct_arr, cnt);			// struct sorting 
				del_hidden(struct_arr,cnt);		// delete hidden
				if(File_cnt!=0 && i!=(argc-1)){	//printf("argv:%s, file_cnt:%d \n", argv[i], File_cnt);		
					continue;}
				//printf("argv:%s \n", argv[i]);
				if(Sflag==1)					// S option, S-sort
					S_array(struct_arr, cnt);

				if(wild_flag==1)				// having wildcard, wildcard print 
					w_print(struct_arr, cnt, "00");
				else
					print(struct_arr, cnt);		// print
			}
		//////////////////////////////////////////////////
			else if(aflag==1 && lflag==1)// -a and -l
			{
				if((save(struct_arr, temp))==-1){// no having file or dir
					fclose(file);				
					return 0;
				}
				if(File_cnt != 0)				// cnt set into File_cnt 
					cnt = File_cnt;
				array(struct_arr, cnt);			// struct sorting
				if(File_cnt!=0 && i!=(argc-1))
					continue;
				if(Sflag==1)					// S option
					S_array(struct_arr, cnt);
				if(wild_flag==1)				// having wildcard, wildcard print 
					w_print(struct_arr, cnt, "11");
				else{
					l_print(struct_arr, cnt);	// struct file -l option printin
				}
			}
		//////////////////////////////////////////////////
			else if(aflag==1 && lflag==0)	// -a
			{
				if((save(struct_arr, temp))==-1){// no having file or dir
					fprintf(file, "</body>\n");	// html body close
					fprintf(file, "</html>\n");	// html close
					fclose(file);				
					return 0;
				}
				if(File_cnt != 0)				// cnt set into File_cnt 
					cnt = File_cnt;

				array(struct_arr, cnt);			// struct sorting
				if(File_cnt!=0 && i!=(argc-1))
					continue;					// S option
				if(Sflag==1)
					S_array(struct_arr, cnt);
				if(wild_flag==1)				// having wildcard, wildcard print
					w_print(struct_arr, cnt, "10");
				else{
					print(struct_arr, cnt);		// print
				}
			}
		//////////////////////////////////////////////////
			else if(aflag==0 && lflag==1)	// -l
			{
				if((save(struct_arr, temp))==-1){	// no having file or dir
					fclose(file);					
					return 0;
				}
				if(File_cnt != 0)				// cnt set into File_cnt wild_path
					cnt = File_cnt;

				array(struct_arr, cnt);			// struct sorting
				del_hidden(struct_arr,cnt);		// delete hidden
				if(File_cnt!=0 && i!=(argc-1))
					continue;
				if(Sflag==1)
					S_array(struct_arr, cnt);	// S option
				if(wild_flag==1)				// having wildcard, wildcard print
					w_print(struct_arr, cnt, "01");
				else
					l_print(struct_arr, cnt);	// struct file -l option printing
			}
		//////////////////////////////////////////////////
			fprintf(file, "</table>\n");		// html table close
			free(struct_arr);					// deallocation

		}
		
	}
	fprintf(file, "</body>\n");					// html body close
	fprintf(file, "</html>\n");					// html close
	fclose(file);								// html file close
	/////////////////////////////////////////////////////////////////////////
	memset(ser_buf, 0, sizeof(char)*100000);		// initializing
	ls_fd=open("html_ls.html", O_RDONLY);		// html file open
	len_out=read(ls_fd, ser_buf, 100000);		// read html file of content to ser_buf 
	close(ls_fd);								// close
	/////////////////////////////////////////////////////////////////////////
	return 0;
}
void l_print(struct data* struct_arr, int cnt)	// l option print
{	
	int  i, j;
	int K=1000;
	int M=1000000;
	int G=1000000000;
	int total=0;
	double value=0;
	int rest=0;
	char* cwd_buf;
	char* cwd;
	char temp_path[100]={0,};
	char hyper_path[200];		// hyperlink path saving variable
	cwd = getcwd(cwd_buf, 1024);	
	if(Dir_path!=NULL && (strcmp(Dir_path, ".")!=0)){		// when Dir_path!=NULL, Dir_path print
		strcpy(temp_path, Dir_path);
		if(temp_path[strlen(temp_path)-1]=='/')
			temp_path[strlen(temp_path)-1]='\0';				
		//printf("Directory path: %s\n", temp_path);		
		fprintf(file, "<h3>");					// Directory path html print
		fprintf(file,"<b>Directory path: %s </b>\n", temp_path);
		fprintf(file, "</h3>");
	}
	else{								// current path print
		strcpy(temp_path, cwd);
		if(temp_path[strlen(temp_path)-1]=='/')
			temp_path[strlen(temp_path)-1]='\0';
	//	printf("Directory path: %s\n", temp_path);		// Directory path html print
		fprintf(file, "<h3>");
		fprintf(file,"<b>Directory path: %s </b>\n", temp_path);
		fprintf(file, "</h3>");
	}
	fprintf(file, "<table border = 2 width = 800>\n");		// table creating, plus option
				
	for(i=0 ; i<cnt ; i++)						// total counting
	{
		if(struct_arr[i].hidden_flag==0)	
			total=total+struct_arr[i].st_blocks;
	}
	
	if(hflag==1){	 //h option of total set			
		if(total>=G){				// If G,
			total=total/2;	
			value=total/G;
			printf("%.1fG	\n", value);
			fprintf(file, "%.1fG	", value);
		}
		else if(total<=G && total>=M){		// If M,
			total=total/2;
			value=total/M;
			printf("%.1fM	\n", value);
			fprintf(file, "%.1fM	", value);
		}
		else if(total<=M && total>=K){		// If K,
			total=total/2;
			value=total/K;
			printf("%.1fK	\n", value);	// value : double type , %.1f
			fprintf(file, "%.1fK	", value); 	
		}
		else{
			printf("total: %d\n", total/2);	// total print
			fprintf(file, "<h3>");
			fprintf(file, "<b>total: %d</b>", total/2);	// total print
			fprintf(file, "</h3>");
		}
		value=0;			
	}	
	else{
		//printf("total: %d\n", total/2);				// total print
		fprintf(file, "<h3>");
		fprintf(file, "<b>total: %d</b>", total/2);
		fprintf(file, "</h3>");
	}
	if(sflag==1){
		fprintf(file, "<tr><th> Block Size </th><th> Name </th><th> 	Permission</th>\n");	// table head print
		fprintf(file, "<th> Link </th> <th> Owner </th> <th> Group </th> ");
		fprintf(file, "<th> Size </th> <th> Last Modified </th></tr> ");
	}
	else{
		fprintf(file, "<tr><th> Name </th><th> Permission </th>\n");
		fprintf(file, "<th> Link </th> <th> Owner </th> <th> Group </th> ");
		fprintf(file, "<th> Size </th> <th> Last Modified </th></tr> ");

	}
	if(wild_flag==0){	// No wild card

		for(i=0 ; i<cnt ; i++)
		{
			if(struct_arr[i].hidden_flag==0)	// except hidden file
			{	
				if(sflag==1){			// s-flag
					printf("%3ld %3c", struct_arr[i].st_blocks/2, 0);
					fprintf(file, "<tr><td> <p align=right> %ld </p> </td>", struct_arr[i].st_blocks/2);				
				}	// table of cell print to align right
				else
					fprintf(file, "<tr>" );
				strcpy(hyper_path, temp_path);
				strcat(hyper_path, "/");
				strcat(hyper_path, struct_arr[i].file_name);
				fprintf(file, "<td> <a href=%s> %s</a><br> </td>\n", hyper_path, struct_arr[i].file_name);// table of cell print to align right	
				////////////////////////////////////////////////////Mecro using, permission print or html print
				if(S_ISDIR(struct_arr[i].st_mode)){ 
					//printf("d");					// directory, "d"
					fprintf(file, "<td><center> d");
				}
	   			else if(S_ISLNK(struct_arr[i].st_mode)){
					//printf("l");	// link "l"
					fprintf(file, "<td><center> l");
				}	   			
				else{
					//printf("-");					// else regular, "-"
					fprintf(file, "<td><center> -");				
				}
				////////////////////////////////////////////////////
	   			if((struct_arr[i].st_mode&S_IREAD)!=0){
					//printf("r");	// user read
					fprintf(file, "r");
				}
	   			else{
					//printf("-");
					fprintf(file, "-");				
				}
	   			if((struct_arr[i].st_mode&S_IWRITE)!=0){
					//printf("w");	// user write
					fprintf(file, "w");
				}
	   			else{
					//printf("-");
					fprintf(file, "-");				
				}
	   			if((struct_arr[i].st_mode&S_IEXEC)!=0){
					//printf("x");	// user exe
					fprintf(file, "x");
				}
	   			else{
					//printf("-");
					fprintf(file, "-");				
				}
				////////////////////////////////////////////////////
	  			if((struct_arr[i].st_mode&S_IREAD>>3)!=0){
					//printf("r");	// gr read
					fprintf(file, "r");				
				}
	   			else{
					//printf("-");
					fprintf(file, "-");
				}
	   			if((struct_arr[i].st_mode&S_IWRITE>>3)!=0){
					//printf("w");	// gr write
					fprintf(file, "w");
				}
	   			else{ 
					//printf("-");
					fprintf(file, "-");
				}
	   			if((struct_arr[i].st_mode&S_IEXEC>>3)!=0){
					//printf("x");	// gr exe
					fprintf(file, "x");
				}
	   			else{
					//printf("-");
					fprintf(file, "-");			
				}
				////////////////////////////////////////////////////
	  			if((struct_arr[i].st_mode&S_IREAD>>6)!=0){
					//printf("r");	// ot read
					fprintf(file, "r</th>");
				}
	  			 else{ 
					//printf("-"); 
					fprintf(file, "-</th>");
				}
	   			if((struct_arr[i].st_mode&S_IWRITE>>6)!=0){
					//printf("w"); // ot write
					fprintf(file, "w</th>");
				}
	   			else{
					//printf("-");
					fprintf(file, "-</th>");
				}
	   			if((struct_arr[i].st_mode&S_IEXEC>>6)!=0){
					//printf("x ");	// ot exe
	  				fprintf(file, "x</center></th>");
				}
				else{
					//printf("- ");
					fprintf(file, "-</center></th>");
				}
				////////////////////////////////////////////////////
				//printf("%3ld	", struct_arr[i].st_nlink);		// each struct_arr file data print
				fprintf(file, "<td> <p align=right> %ld </p></td>", struct_arr[i].st_nlink);
				//printf("%-5s", struct_arr[i].us_name);
				fprintf(file, "<td><center> %s </center></td>", struct_arr[i].us_name);	// table of cell print to align center
				//printf("%-10s", struct_arr[i].gr_name);
				fprintf(file, "<td><center> %s </center></td>", struct_arr[i].gr_name);
				if(hflag==1){	 //h option
							
					if(struct_arr[i].st_size>=G){			// If G
						value=struct_arr[i].st_size/G;
						printf("%.1fG	", value);
						fprintf(file, "<td><p align=right> %.1fG </p></td>", value); // table of cell print to align right	
					}
					else if(struct_arr[i].st_size<=G && struct_arr[i].st_size>=M){
						value=struct_arr[i].st_size/M;		// If M
						printf("%.1fM	", value);
						fprintf(file, "<td><p align=right> %.1fM </p></td>", value);
					}
					else if(struct_arr[i].st_size<=M && struct_arr[i].st_size>=K){
						value=struct_arr[i].st_size/K;		// If K
						printf("%.1fK	", value);	// print
						fprintf(file, "<td><p align=right> %.1fK </p></td>", value);
					}
						
					else{						// low number
						printf("%ld	", struct_arr[i].st_size);
						fprintf(file, "<td><p align=right> %ld </p></td>", struct_arr[i].st_size);
					}
					value=0;
				}	
				else{	// no option
					//printf("%10ld", struct_arr[i].st_size);
					fprintf(file, "<td><p align=right> %ld </p></td>", struct_arr[i].st_size); // table of cell print to align right	
				}
				//printf("%4d", struct_arr[i].tm_mon+1);
				fprintf(file, "<td><center> %d ", struct_arr[i].tm_mon+1);
				//printf("%4d %3c", struct_arr[i].tm_mday, 0);
				fprintf(file, " %d ", struct_arr[i].tm_mday);
				////////////////////////////////////////////////////
				if(struct_arr[i].tm_hour<10){				// time struct_arr time data print
					//printf("0%d:", struct_arr[i].tm_hour);
					fprintf(file, " 0%d:", struct_arr[i].tm_hour);				
				}
				else{
					//printf("%2d:", struct_arr[i].tm_hour);
					fprintf(file, " %2d:", struct_arr[i].tm_hour);
				}
				if(struct_arr[i].tm_min<10){			
					//printf("0%d ", struct_arr[i].tm_min);
					fprintf(file, "0%d </center></td>", struct_arr[i].tm_min); // table of cell print to align right	
				}
				else{
					//printf("%d ", struct_arr[i].tm_min);
					fprintf(file, "%d </center></td>", struct_arr[i].tm_min);  // table of cell print to align right		 			
				}
				//printf("%s\n", struct_arr[i].file_name);
				////////////////////////////////////////////////////	
			}
			memset(hyper_path, '0', sizeof(char)*200);
		
		}
	}
	else{		// wild card, almost samething function upper code 
		for(i=0 ; i<cnt ; i++)
		{
			if(struct_arr[i].hidden_flag==0)	// except hidden file
			{	
				if(struct_arr[i].w_flag==1){	// if having wildcard flag
					
					if(struct_arr[i].d_flag==0){	// if no having directory flag
						if(sflag==1){			// s-flag
							printf("%3ld %3c", struct_arr[i].st_blocks/2, 0);
							fprintf(file, "<tr><td> <p align=right> %ld </p> </td>", struct_arr[i].st_blocks/2);				
						}
						else
							fprintf(file, "<tr>" );

						strcpy(hyper_path, temp_path);
						strcat(hyper_path, "/");
						strcat(hyper_path, struct_arr[i].file_name);
						fprintf(file, "<td> <a href=%s> %s</a><br> </td>\n", hyper_path, struct_arr[i].file_name);		
					////////////////////////////////////////////////////Mecro using, permission print
						if(S_ISDIR(struct_arr[i].st_mode)){ 
							printf("d");		// directory, "d"
							fprintf(file, "<td><center> d");
						}
			   			else if(S_ISLNK(struct_arr[i].st_mode)){
							printf("l");	// link "l"
							fprintf(file, "<td><center> l");
						}	   			
						else{
							printf("-");					// else regular, "-"
							fprintf(file, "<td><center> -");				
						}
					////////////////////////////////////////////////////
			   			if((struct_arr[i].st_mode&S_IREAD)!=0){
							printf("r");	// user read
							fprintf(file, "r");
						}
			   			else{
							printf("-");
							fprintf(file, "-");				
						}
			   			if((struct_arr[i].st_mode&S_IWRITE)!=0){
							printf("w");	// user write
							fprintf(file, "w");
						}
			   			else{
							printf("-");
							fprintf(file, "-");				
						}
			   			if((struct_arr[i].st_mode&S_IEXEC)!=0){
							printf("x");	// user exe
							fprintf(file, "x");
						}
			   			else{
							printf("-");
							fprintf(file, "-");				
						}
						////////////////////////////////////////////////////
			  			if((struct_arr[i].st_mode&S_IREAD>>3)!=0){
							printf("r");	// gr read
							fprintf(file, "r");				
						}
			   			else{
							printf("-");
							fprintf(file, "-");
						}
			   			if((struct_arr[i].st_mode&S_IWRITE>>3)!=0){
							printf("w");	// gr write
							fprintf(file, "w");
						}
	   					else{ 
							printf("-");
							fprintf(file, "-");
						}
	   					if((struct_arr[i].st_mode&S_IEXEC>>3)!=0){
							printf("x");	// gr exe
							fprintf(file, "x");
						}
	   					else{
							printf("-");
							fprintf(file, "-");			
						}
						////////////////////////////////////////////////////
	  					if((struct_arr[i].st_mode&S_IREAD>>6)!=0){
							printf("r");	// ot read
							fprintf(file, "r</th>");
						}
	  					 else{ 
							printf("-"); 
							fprintf(file, "-</th>");
						}
	   					if((struct_arr[i].st_mode&S_IWRITE>>6)!=0){
							printf("w"); // ot write
							fprintf(file, "w</th>");
						}
	   					else{
							printf("-");
							fprintf(file, "-</th>");
						}
	   					if((struct_arr[i].st_mode&S_IEXEC>>6)!=0){
							printf("x ");	// ot exe
	  						fprintf(file, "x</center></th>");
						}
						else{
							printf("- ");
							fprintf(file, "-</center></th>");
						}
						////////////////////////////////////////////////////
						printf("%3ld	", struct_arr[i].st_nlink);		// each struct_arr file data print
						fprintf(file, "<td> <p align=right> %ld </p></td>", struct_arr[i].st_nlink);
						printf("%-5s", struct_arr[i].us_name);
						fprintf(file, "<td><center> %s </center></td>", struct_arr[i].us_name);
						printf("%-10s", struct_arr[i].gr_name);
						fprintf(file, "<td><center> %s </center></td>", struct_arr[i].gr_name);
						if(hflag==1){	 //h option
									
							if(struct_arr[i].st_size>=G){			// If G
								value=struct_arr[i].st_size/G;
								printf("%.1fG	", value);
								fprintf(file, "<td><p align=right> %.1fG </p></td>", value);
							}
							else if(struct_arr[i].st_size<=G && struct_arr[i].st_size>=M){
								value=struct_arr[i].st_size/M;		// If M
								printf("%.1fM	", value);
								fprintf(file, "<td><p align=right> %.1fM </p></td>", value);
							}
							else if(struct_arr[i].st_size<=M && struct_arr[i].st_size>=K){
								value=struct_arr[i].st_size/K;		// If K
								printf("%.1fM	", value);	// print
								fprintf(file, "<td><p align=right> %.1fK </p></td>", value);
							}
						
							else{						// low number
								printf("%ld	", struct_arr[i].st_size);
								fprintf(file, "<td><p align=right> %ld </p></td>", struct_arr[i].st_size);
							}
					
						}	
						else{	// no option
							printf("%2ld", struct_arr[i].st_size);
							fprintf(file, "<td><p align=right> %ld </p></td>", struct_arr[i].st_size);
						}
						printf("%5d", struct_arr[i].tm_mon+1);
						fprintf(file, "<td><center> %d ", struct_arr[i].tm_mon+1);
						printf("%5d	", struct_arr[i].tm_mday);
						fprintf(file, " %d ", struct_arr[i].tm_mday);
						////////////////////////////////////////////////////
						if(struct_arr[i].tm_hour<10){				// time struct_arr time data print
							printf("0%d:", struct_arr[i].tm_hour);
							fprintf(file, " 0%d:", struct_arr[i].tm_hour);				
						}
						else{
							printf("%d:", struct_arr[i].tm_hour);
							fprintf(file, " %d:", struct_arr[i].tm_hour);
						}
						if(struct_arr[i].tm_min<10){			
							printf("0%d ", struct_arr[i].tm_min);
							fprintf(file, "0%d </center></td>", struct_arr[i].tm_min);
						}
						else{
							printf("%d ", struct_arr[i].tm_min);
							fprintf(file, "%d </center></td>", struct_arr[i].tm_min);				
						}
						printf("%s\n", struct_arr[i].file_name);
				////////////////////////////////////////////////////
						}
				}	
			}
			memset(hyper_path, '0', sizeof(char)*200);		// initializing
		}
	}
}
int save(struct data* struct_arr, char* G)
{
	int i=0;
	int j=0;
	int cnt=0;
	int flag=0;						// file_temp exist flag
	int jumflag=0;
	struct stat buf;					// stat struct
	struct tm* time;					// tm struct 
	DIR *dirp;						
	struct dirent *dir;					// dirent struct
	char abs_path[100]={0,};

	if(G == NULL)	G = ".";	// if G of path is NULL, current dir 
	else
		jumflag=1;		// jumflag set
	dirp=opendir(G);		// it counts directory entry files in dirp
	if(dirp == NULL){		// when is'0', error
		printf("ls: cannot access %s: No such file or directory\n", G);
		return -1;
	}	
	while((dir=readdir(dirp))!=NULL){	
		if(strstr(dir->d_name, ".html")!=NULL){
			if(html_flag==0){
				continue;
			}
			cnt++;
		}
		else
			cnt++;
	}
	closedir(dirp);
								// no having file in instruction
	if((G[0]!='/' && jumflag==0 && G[0]!='-') || Dirfile_flag==1){
		dirp=opendir(G);				// G path open
		while((dir=readdir(dirp))!=NULL)		// everything of struct file save file data
		{		
			if(strstr(dir->d_name, ".html")!=NULL){
				if(html_flag==0){
					continue;
				}
				cnt++;
			}
			else
				cnt++;
			memset(abs_path, 0, sizeof(char)*100);	
			strcpy(abs_path, G);
			strcat(abs_path, "/");
			strcat(abs_path, dir->d_name);
		/////////////////////////////////////////////////////////////////	
			stat(abs_path, &buf);			// i struct_arr saving to data
			
			for(j=0; j<File_cnt; j++){
				if(strcmp(dir->d_name, File_temp[j]) == 0){	// when file_temp exist
					flag=1;					// flag set
				/////////////////////////////////////////////////////////////////
					strcpy(struct_arr[i].file_name, dir->d_name);	// i struct_arr saving to data
					struct_arr[i].st_mode = buf.st_mode;
					struct_arr[i].st_nlink = buf.st_nlink;
					struct_arr[i].st_blocks = buf.st_blocks;
					struct_arr[i].st_size = buf.st_size;
					struct_arr[i].us_name = getpwuid(buf.st_uid)->pw_name;
					struct_arr[i].gr_name = getgrgid(buf.st_gid)->gr_name;
					time=localtime(&buf.st_mtime);
					struct_arr[i].tm_min = time->tm_min;
					struct_arr[i].tm_hour = time->tm_hour;
					struct_arr[i].tm_mday = time->tm_mday;
					struct_arr[i].tm_mon = time->tm_mon;
				/////////////////////////////////////////////////////////////////
					i++;
					break;
				}
			}
			if(i+1 == cnt && flag==0){				// flag 0, when cnt== i+1, no having dir
				printf("ls: cannot access %s: No such file or directory", G);
				return -1;
			}
		}
	}
	else{	// having file in instruction
		dirp=opendir(G);
		i=0; 
		while((dir=readdir(dirp))!=NULL)
		{	
			if(strstr(dir->d_name, ".html")!=NULL){
				if(html_flag==0){
					continue;
				}
				cnt++;
			}
			else
				cnt++;
			memset(abs_path, 0, sizeof(char)*100);	
			strcpy(abs_path, G);

			if(abs_path[strlen(abs_path)-1]=='/'){	// if it remains '/' strcat dir of name
				strcat(abs_path, dir->d_name);
			}
			else{					// else
				strcat(abs_path, "/");		// add '/'
				strcat(abs_path, dir->d_name);
			}
		/////////////////////////////////////////////////////////////////	
			stat(abs_path, &buf);		// i struct_arr saving to data
			strcpy(struct_arr[i].file_name, dir->d_name);
			struct_arr[i].st_mode = buf.st_mode;
			struct_arr[i].st_nlink = buf.st_nlink;
			struct_arr[i].st_blocks = buf.st_blocks;
			struct_arr[i].st_size = buf.st_size;
			struct_arr[i].us_name = getpwuid(buf.st_uid)->pw_name;	
			struct_arr[i].gr_name = getgrgid(buf.st_gid)->gr_name;
			time=localtime(&buf.st_mtime);
			struct_arr[i].tm_min = time->tm_min;
			struct_arr[i].tm_hour = time->tm_hour;
			struct_arr[i].tm_mday = time->tm_mday;
			struct_arr[i].tm_mon = time->tm_mon;
		/////////////////////////////////////////////////////////////////
			if(wild_flag==1)	// if wild flag 1
			{	
				if(struct_arr[i].file_name[0]!='.'){ 
					if(S_ISDIR(struct_arr[i].st_mode)) // if it directory
						struct_arr[i].d_flag=1;	
				if(fnmatch(wild_str, dir->d_name, 0)==0)   // if searching thought wildcard 
					struct_arr[i].w_flag=1;	
				}
				if(struct_arr[i].wd_flag==1)		// if driectory of wildcard
					struct_arr[i].w_flag=1;		// for struct2_arr in w_print
					//printf("save!!%s\n", struct_arr[i].file_name);
			}
			i++;
		}
	}
	
	closedir(dirp);		// close dirp
	return 0;		// exit
	
}
void print(struct data* struct_arr, int cnt)					// print function
{
		int j=0;
		int total=0;
		char cwd[200];
		char hyper_path[200];						// hyper path variable
		getcwd(cwd, 1024);
		if(Dir_path!=NULL){						// path decision
			memset(cwd, '0', sizeof(char)*200);
			strcpy(cwd,Dir_path);
		}
		fprintf(file, "<h3>");						// directory path print
		fprintf(file, "<b>Directory path: %s</b>\n", cwd);
		fprintf(file, "</h3>");
		fprintf(file, "<table border = 2 width = 300>\n");		// table creating with option

		if(sflag==1){
			for(j=0 ; j<cnt ; j++)					// total counting
			{
				if(struct_arr[j].hidden_flag==0)	
					total=total+struct_arr[j].st_blocks;
			}
			printf("total: %d\n",total/2);				
			fprintf(file, "<h3>");					// total value print to html file
			fprintf(file, "<b>total: %d</b>\n",total/2);
			fprintf(file, "</h3>");
			fprintf(file, "<tr><th> Block size </th><th> File Name </th></tr>\n");	// table head print to html file
			for(j=0 ; j<cnt; j++){					// struct file printing 
				if(struct_arr[j].hidden_flag!=1){		// if hidden_flag==0
					
					printf("%ld	", struct_arr[j].st_blocks/2);
					printf("%s\n",struct_arr[j].file_name);
					strcpy(hyper_path, cwd);
					strcat(hyper_path, "/");
					strcat(hyper_path, struct_arr[j].file_name);
					fprintf(file, "<tr><td> %ld	 </td> <td> <a href=%s> %s</a><br> </td></tr>\n",struct_arr[j].st_blocks/2, hyper_path, struct_arr[j].file_name);				// print file name to file with hyperlink and data
					memset(hyper_path, '0', sizeof(char)*200);
				}
			}
		}
		else{
			fprintf(file, "<tr><th> File Name </th></tr>\n");	// print file name to file
			for(j=0 ; j<cnt; j++)					// struct file printing 
			{
				if(struct_arr[j].hidden_flag!=1){		// if hidden_flag==0
					strcpy(hyper_path, cwd);
					strcat(hyper_path, "/");
					strcat(hyper_path, struct_arr[j].file_name);
					printf("%s\n",struct_arr[j].file_name);
					fprintf(file, "<tr><td> <a href=%s> %s</a><br> </td></tr>\n", hyper_path, struct_arr[j].file_name);		
					memset(hyper_path, '0', sizeof(char)*200);		// print file name to file with hyperlink and data			
				}	
			}
		}	
}
void w_print(struct data* struct_arr, int cnt, char* al_flag)	// print file or dir included in wildcard func
{
	DIR *dirp;						
	struct dirent *dir;					// dirent struct
	struct data* struct_arr2;
	int i, j;
	char save_path[100]={0,};				// temp path array
	char temp[100]={0,};
	char hyper_path[200]={0,};
	char* cwd_buf;						// for getcwd systemcall
	char* cwd;
	int w_cnt=0;						// wildcard file or dir count
	int total=0;
	
	if(wild_path==NULL){					// path decide
		cwd = getcwd(cwd_buf, 1024);
		wild_path=cwd;					// current path
	}
	fprintf(file, "<table border = 2 width = 800>\n"); 	// table creating with option

	if( (strcmp(al_flag, "00")==0) || (strcmp(al_flag, "10")==0) ){		// -a = 0 , -l = 0 || -a = 1, -l = 0
		Dir_path=wild_path;				// path modify
		printf("\nDirectory path: %s\n", wild_path);	// wild_path directory path print to html file
		fprintf(file, "<h3>");
		fprintf(file, "<b>Directory path: %s</b>", wild_path);
		fprintf(file, "</h3>");		
		if(sflag==1){	// s option
			for(i=0 ; i<cnt ; i++)	
			{
				if(struct_arr[i].hidden_flag!=1 && struct_arr[i].w_flag==1 && struct_arr[i].d_flag==0)	
					total=total+struct_arr[i].st_blocks;
			}
			printf("total: %d\n",total/2);		// total printing
			fprintf(file, "<h3>");
			fprintf(file, "<b>total: %d</b>\n",total/2);
			fprintf(file, "</h3>");			
			fprintf(file, "<tr><th> Block size </th><th> File Name </th></tr>\n");	// s option table head print
			for(i=0 ; i<cnt; i++){						// when d_flag=0, print wildcard file
				if(struct_arr[i].hidden_flag!=1 && struct_arr[i].w_flag==1 && struct_arr[i].d_flag==0){
					printf("%ld	", struct_arr[i].st_blocks/2);
					printf("%s\n",struct_arr[i].file_name);
					fprintf(file, "<tr><td> %ld </td>\n", struct_arr[i].st_blocks/2);
					fprintf(file, "<td> %s </td></tr>\n", struct_arr[i].file_name);		
				}
				memset(hyper_path, '0', sizeof(char)*200);		
			}
		}
		else{		// no s option
			
			fprintf(file, "<tr><th> File Name </th></tr>\n");
			for(i=0 ; i<cnt; i++)						// struct file printing 
			{
				if(struct_arr[i].hidden_flag!=1 && struct_arr[i].w_flag==1 && struct_arr[i].d_flag==0){
					printf("%s\n",struct_arr[i].file_name);		// of wild_card, no directory
					strcpy(hyper_path, wild_path);
					strcat(hyper_path, "/");
					strcat(hyper_path, struct_arr[i].file_name);
					fprintf(file, "<tr><td> <a href=%s> %s</a><br> </td></tr>\n", hyper_path, struct_arr[i].file_name);				
				}	// print file name to file with hyperlink and data	
				memset(hyper_path, '0', sizeof(char)*200);	
			}
		}	
		
	} 
	else if((strcmp(al_flag, "01")==0) || (strcmp(al_flag, "11")==0) ){ 	// -a = 0, -l = 1 // -a = 1, -l = 1
		Dir_path=wild_path;				// path modify
		l_print(struct_arr, cnt);			// l_print call
	}
	strcpy(save_path, wild_path);				// save_path re-saving
	for(i=0 ; i<cnt; i++)							
	{						// when d_flag=1, print wildcard file of dir
		if(struct_arr[i].hidden_flag==0 && struct_arr[i].w_flag==1 && struct_arr[i].d_flag==1){	
			fprintf(file, "<p> </table>");		// table close
			strcpy(temp, struct_arr[i].file_name);
			strcat(save_path, "/");			// string cat '/'
			strcat(save_path, temp);		// string cat dir name
			memset(temp, '0', sizeof(char)*100);
			dirp=opendir(save_path);		// open!!
			if(dirp==NULL)
				break; 

			printf("Directory path: %s\n", struct_arr[i].file_name);	// no having dir
			fprintf(file, "<h3>");
			if(lflag!=1)			
				fprintf(file, "<b>Directory path: %s</b>", save_path);	// directory path printing
			fprintf(file, "</h3>");
			fprintf(file, "<table border = 2 width = 800>\n");		// table creating
			
			while((dir=readdir(dirp))!=NULL){
				if(strstr(dir->d_name, ".html")!=NULL){
					if(html_flag==0){
						continue;
					}
					cnt++;
				}
				else
					cnt++;
			}

			closedir(dirp);
	
			Dir_path=save_path;
			struct_arr2=(struct data*)malloc(sizeof(struct data)*w_cnt);// struct array allcation 
			memset(struct_arr2, 0, sizeof(struct data)*w_cnt);
			for(j=0 ; j<w_cnt ; j++)
				memset(struct_arr2[j].file_name, 0, sizeof(char)*100);
			for(j=0 ; j<w_cnt ; j++)				// wd_flag.. it dir of content for print 
				struct_arr2[j].wd_flag=1;
			save(struct_arr2, save_path);				// data inserting
			array(struct_arr2, w_cnt);				// struct sorting
			if(w_cnt>2 && lflag!=1)
				fprintf(file, "<tr><th> File Name </th></tr>\n");	// file name table print to html file
			if( (strcmp(al_flag, "00")==0) || (strcmp(al_flag, "01")==0) ){
				del_hidden(struct_arr2, w_cnt);			// delete hidden
			}

			if( (strcmp(al_flag, "00")==0)){			// no option
				for(j=0 ; j<w_cnt ; j++){
					if(struct_arr2[j].hidden_flag!=1){	// no having hidden
						printf("%s\n",struct_arr2[j].file_name);
						strcpy(hyper_path, wild_path);
						strcat(hyper_path, "/");
						strcat(hyper_path, struct_arr[j].file_name);
						fprintf(file, "<tr><td> <a href=%s> %s</a><br> </td>\n", hyper_path, struct_arr[j].file_name);
						// print file name to file with hyperlink and data	
					}
					memset(hyper_path, '0', sizeof(char)*200);
				}
		
			}
			else if( (strcmp(al_flag, "10")==0) ){	// -a option
				for(j=0 ; j<w_cnt ; j++){
					printf("%s\n",struct_arr2[j].file_name);
					//fprintf(file,"<tr><td>%s</th></td>\n",struct_arr2[j].file_name);
					strcpy(hyper_path, wild_path);
					strcat(hyper_path, "/");
					strcat(hyper_path, struct_arr[j].file_name);
					fprintf(file, "<tr><td> <a href=%s> %s</a><br> </td>\n", hyper_path, struct_arr[j].file_name);	// print file name to file with hyperlink and data	
				}
				memset(hyper_path, '0', sizeof(char)*200);	// initializing hyper path
			}					// -l or -al option
			else if( (strcmp(al_flag, "01")==0) || (strcmp(al_flag, "11")==0) )	
				l_print(struct_arr2, w_cnt);	// -l print func
			
			printf("\n");			// end line
			free(struct_arr2);		//deallocation
			w_cnt=0;			// initializing for re-use
			strcpy(save_path, wild_path);
			wild_flag=0;
		}
		
	}
	fprintf(file, "<p> </table>");			// table close
}
void S_array(struct data* struct_arr, int cnt)
{
	int i, j;
	struct data temp;				// temporary struct
	int arr[200]={0,};				// temporary arr
	for(i=0 ; i< cnt ; i++)	
		arr[i]=struct_arr[i].st_blocks/2;	// all ofblock saving
	
	for(i=0 ; i< cnt ; i++)
	{	
		for(j=i ; j<cnt ; j++)
		{
			if(struct_arr[i].st_blocks/2<=struct_arr[j].st_blocks/2){
				temp=struct_arr[i];	// swap of sorting
				struct_arr[i]=struct_arr[j];
				struct_arr[j]=temp;
			}
		}
	}
}
void array(struct data* struct_arr, int cnt)	// sorting func
{
	int i, j, k;
	struct data temp;		// temporary struct
	char temp1[200]={0,};		// temporary arr1
	char temp2[200]={0,};		// temporary arr2
	for(i=0 ; i< cnt ; i++)
	{	
		for(j=i ; j<cnt ; j++)
		{
			strcpy(temp1, struct_arr[i].file_name);	// temp1 saving
			//////////////////////////////////////////////////////////// temp1
			if((temp1[0]=='.')&&((temp1[1]!='.')&&((temp1)[1]!=0 )))
			{					// when is hiddien 
				
				for(k=0 ; struct_arr[i].file_name[k]!='\0' ; k++)
					temp1[k]=struct_arr[i].file_name[k+1];	// saving to plus 1 index file_name into temp
			}
			else						// no hidden
				strcpy(temp1, struct_arr[i].file_name);	// original saving
			for(k=0 ; temp1[k]!='\0' ; k++)		// when is captical alpahabet 
			{
				if(temp1[k]>=65 && temp1[k]<=90)	// changing to askll
					temp1[k]+=32;
			}
			//////////////////////////////////////////////////////////// temp2 is samething with temp1
			strcpy(temp2, struct_arr[j].file_name);		
			if((temp2[0]=='.')&&((temp2[1]!='.')&&((temp2)[1]!='\0' )))
			{				
				for(k=0 ; struct_arr[j].file_name[k]!='\0' ; k++)
					temp2[k]=struct_arr[j].file_name[k+1];
			}
			else
				strcpy(temp2, struct_arr[j].file_name);
			for(k=0 ; temp2[k]!='\0' ; k++)
			{
				if(temp2[k]>=65 && temp2[k]<=90)
					temp2[k]+=32;
			}
			////////////////////////////////////////////////////////////
			if(strcmp(temp1, temp2)>=0)			// compare to, if temp1 >= temp2
			{
				temp=struct_arr[i];			// swap
				struct_arr[i]=struct_arr[j];
				struct_arr[j]=temp;
			}
			memset(temp1, 0, sizeof(char)*200);		// temp1, temp2 initializing
			memset(temp2, 0, sizeof(char)*200);		
		}
	}
		
}
int del_hidden(struct data* struct_arr, int cnt)			// delete hidden
{
	int i=0;

	for(i=0 ; i<cnt ; i++)					
	{
		if(struct_arr[i].file_name=="."||struct_arr[i].file_name==".."||struct_arr[i].file_name[0]=='.'){ // if having '.', "..", hid..
			struct_arr[i].hidden_flag=1;			// flag=1
		}
	}
	return 0;
}
char* back_strtok(char *str)						// back strtoken
{
	int i;
	int length=0;
	length=strlen(str);
	for(i=length ; i>=0 ; i--){
		if(str[i]=='/'){
			str[i]=0;					// if '/', save NULL
			break;
		}
	}
   return &str[i+1];							// cut string
}
