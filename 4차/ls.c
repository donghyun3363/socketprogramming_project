#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>		
#include <string.h>
#include <stdlib.h>		
#include <pwd.h>
#include <grp.h>
#include <time.h>
///////////////////////////////////////////////// Header file
struct data			// file data struct
{
	char* file_name;	// file name saving
	int hidden_flag;	// flag saving if it is hidden file 
	char* us_name;	
	char* gr_name;
	mode_t st_mode;
	nlink_t st_nlink;
	off_t st_size;
	blkcnt_t st_blocks;
	
	int wd_flag;		// included in wildcard of directory flag 
	int w_flag;		// wildcard flag
	int d_flag;		// drectory flag
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
};
//////////////////////////////////////////////////
void array(struct data* struct_arr, int cnt);		// sort func
int save(struct data* struct_arr, char* G);		// data save into structure func
int del_hidden(struct data* struct_arr, int cnt);	// hidden structure delete func
void l_print(struct data* struct_arr, int cnt);		// l option print func
char* back_strtok(char *str);				// strtok of back func
void w_print(struct data* struct_arr, int cnt, char* al_flag);	// if wildcard, print func
void S_array(struct data* struct_arr, int cnt);		// S option sort func
int File_cnt;						// file count
char File_temp[100][100];				// file saving 
char* Dir_path;						// path saving
char* wild_path;					// wild card path
int Dirfile_flag;					// file in dir path flag
int wild_flag;						// wild card flag
char* wild_str;						// wildcard string
int hflag;						// h option flag
int sflag;						// s option flag
int Sflag;						// S option flag
//////////////////////////////////////////////////
int main(int argc, char** argv)
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
	int lflag=0;
	hflag=0;		// initializing
	sflag=0;
	Sflag=0;
	File_cnt=0;		
	Dir_path=NULL;
	Dirfile_flag=0;
	wild_flag=0;
	wild_path=NULL;
	wild_str=NULL;
	//////////////////////////////////////////////////
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
				return 0;
			default:		// else
				break;	
		}
	}
	if(argc<2)						// just entering instruction
	{	////////////////////////////////////////////////// current dir reading
		dirp=opendir(".");
		while((dir=readdir(dirp))!=NULL)
			cnt++;
		closedir(dirp);
		//////////////////////////////////////////////////
		struct_arr=(struct data*)malloc(sizeof(struct data)*cnt);	// struct array allocation
		memset(struct_arr, 0, sizeof(struct data)*cnt);
		//////////////////////////////////////////////////
		if((save(struct_arr, "."))==-1)					// if empty 
			return 0;
		array(struct_arr, cnt);						// struct sorting
		del_hidden(struct_arr,cnt);					// hidden delete
		for(j=0 ; j<cnt; j++)						// printing struct
		{
			if(struct_arr[j].hidden_flag!=1)
				printf("%s\n",struct_arr[j].file_name);
		}
		free(struct_arr);						// deallocation
		return 0;		
	}		
	//////////////////////////////////////////////////
	for(i=1 ; i<argc ; i++)
	{
		cnt=0;			// initializing
		temp=NULL;
		Dir_path=NULL;
		Dirfile_flag=0;
		token=NULL;
		wild_path=NULL;
		wild_str=NULL;
		wild_flag=0;
		//////////////////////////////////////////////////
		if(argv[i][0]!='-'||(i == (argc-1)))				// No option || Nothing included in option
		{

			if(strstr(argv[i], "*")!=NULL || strstr(argv[i], "?") || strstr(argv[i], "[")){	
				wild_flag=1;			// at least, when is existing wildcard
				wild_str=argv[i];
				if(argv[i][0]!='/')		// if path
					temp=".";
				else{
					temp=argv[i];
					token=back_strtok(temp);
					wild_str=token;		// wildcard string
					wild_path=temp;		// just path
				}
				
			}			
			else if(argv[i][0]!='/' && argv[i][0]!='.'&&argv[i][0]!='-'){
				strcpy(File_temp[File_cnt], argv[i]);			// save file name into 2D array 
				File_cnt++;
				temp=NULL;	
			}
			else if(argv[i][0]=='-'){					// just having option in istr
				temp=".";
			}
			else { 								// rest of path in temp
				temp=argv[i];
			}
			////////////////////////////////////////////////////////////////////////////////////////////from now temp decide.
			if(temp == NULL){					// no dir name in instruction
				dirp=opendir(".");				// current dir open		
			}
			else{										
				dirp=opendir(temp);				// dir name open
				Dir_path=temp;					// saving the path

				if(dirp==NULL){				 // Dir+file case or wild case
					
					token=back_strtok(temp);		// token file	
					dirp=opendir(temp);			// token path save to dirp

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
			if(dirp == NULL && token==NULL){				// when is'0', error
				printf("ls: cannot access %s: No such file or directory\n", temp);
				continue;
			} 
			while((dir=readdir(dirp))!=NULL)				// dir count
				cnt++;
			closedir(dirp);
			struct_arr=(struct data*)malloc(sizeof(struct data)*cnt);	// struct array allcation 
			memset(struct_arr, 0, sizeof(struct data)*cnt);	
			////////////////////////////////////////////////////////////////////////////////////////////
			if(aflag==0 && lflag==0)// nothing -a, and -lDir_flag
			{	
				
				if((save(struct_arr, temp))==-1)		// no having file or dir
					return 0; 
				if(File_cnt != 0)				// cnt set into File_cnt 
					cnt = File_cnt;
				array(struct_arr, cnt);				// struct sorting 
				del_hidden(struct_arr,cnt);			// delete hidden
				if(File_cnt!=0 && i!=(argc-1))			
					continue;
				if(Sflag==1)					// S option, S-sort
					S_array(struct_arr, cnt);
				if(wild_flag==1)				// having wildcard, wildcard print 
					w_print(struct_arr, cnt, "00");
				else{
					if(sflag==1){				// s option,
						for(j=0 ; j<cnt ; j++)				// total counting
						{
							if(struct_arr[j].hidden_flag==0)	
							total=total+struct_arr[j].st_blocks;
						}
						printf("total: %d\n",total/2);
						for(j=0 ; j<cnt; j++){				// struct file printing 
							if(struct_arr[j].hidden_flag!=1){
								printf("%ld	", struct_arr[j].st_blocks/2);
								printf("%s\n",struct_arr[j].file_name);
							}
						}
					}
					else{
						for(j=0 ; j<cnt; j++){				// struct file printing 
							if(struct_arr[j].hidden_flag!=1)
								printf("%s\n",struct_arr[j].file_name);
						}
					}
				}
			}
		//////////////////////////////////////////////////
			else if(aflag==1 && lflag==1)// -a and -l
			{
				//printf("cnt:%d\n",cnt);
				if((save(struct_arr, temp))==-1)		// no having file or dir
					return 0;
				if(File_cnt != 0)				// cnt set into File_cnt 
					cnt = File_cnt;
				array(struct_arr, cnt);				// struct sorting
				if(File_cnt!=0 && i!=(argc-1))
					continue;
				if(Sflag==1)					// S option
					S_array(struct_arr, cnt);
				if(wild_flag==1)				// having wildcard, wildcard print 
					w_print(struct_arr, cnt, "11");
				else
					l_print(struct_arr, cnt);		// struct file -l option printing
			}
		//////////////////////////////////////////////////
			else if(aflag==1 && lflag==0)	// -a
			{
				if((save(struct_arr, temp))==-1)		// no having file or dir
					return 0;
				if(File_cnt != 0)				// cnt set into File_cnt 
					cnt = File_cnt;

				array(struct_arr, cnt);				// struct sorting
				if(File_cnt!=0 && i!=(argc-1))
					continue;				// S option
				if(Sflag==1)
					S_array(struct_arr, cnt);
				if(wild_flag==1)				// having wildcard, wildcard print
					w_print(struct_arr, cnt, "10");
				else{
					if(sflag==1){
						for(j=0 ; j<cnt ; j++)				// total counting
						{
							if(struct_arr[j].hidden_flag==0)	
							total=total+struct_arr[j].st_blocks;
						}
						printf("total: %d\n",total/2);
						for(j=0 ; j<cnt; j++){				// struct file printing 
							if(struct_arr[j].hidden_flag!=1){
								printf("%ld	", struct_arr[j].st_blocks/2);
								printf("%s\n",struct_arr[j].file_name);
							}
						}
					}
					else{
						for(j=0 ; j<cnt; j++)				// struct file printing 
						{
							if(struct_arr[j].hidden_flag!=1)
								printf("%s\n",struct_arr[j].file_name);
						}
					}
				}
			}
		//////////////////////////////////////////////////
			else if(aflag==0 && lflag==1)	// -l
			{
				if((save(struct_arr, temp))==-1)		// no having file or dir
					return 0;
				if(File_cnt != 0)				// cnt set into File_cnt wild_path
					cnt = File_cnt;

				array(struct_arr, cnt);				// struct sorting
				del_hidden(struct_arr,cnt);			// delete hidden
				if(File_cnt!=0 && i!=(argc-1))
					continue;
				if(Sflag==1)
					S_array(struct_arr, cnt);		// S option
				if(wild_flag==1)				// having wildcard, wildcard print
					w_print(struct_arr, cnt, "01");
				else
					l_print(struct_arr, cnt);		// struct file -l option printing
			}
		//////////////////////////////////////////////////
			free(struct_arr);	// deallocation

		}
		
	}
	return 0;// exit
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
	if(dirp == NULL){					// when is'0', error
		printf("ls: cannot access %s: No such file or directory\n", G);
		return -1;
	}	
	while((dir=readdir(dirp))!=NULL)
		cnt++;	
	closedir(dirp);
								// no having file in instruction
	if((G[0]!='/' && jumflag==0 && G[0]!='-') || Dirfile_flag==1){
		dirp=opendir(G);				// G path open
		while((dir=readdir(dirp))!=NULL)		// everything of struct file save file data
		{		
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
					struct_arr[i].file_name = dir->d_name;	// i struct_arr saving to data
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
			struct_arr[i].file_name = dir->d_name;
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
			}
			i++;
		}
	}
	
	closedir(dirp);		// close dirp
	return 0;		// exit
	
}
void l_print(struct data* struct_arr, int cnt)	// l option print
{	
	int  i, j;
	int M=1000;
	int K=1000000;
	int G=1000000000;
	int total=0;
	int value=0;
	int rest=0;
	char* cwd_buf;
	char* cwd;
	char temp_path[100]={0,};

	cwd = getcwd(cwd_buf, 1024);					
	if(Dir_path!=NULL && (strcmp(Dir_path, ".")!=0))		// when Dir_path!=NULL, Dir_path print 
		printf("Directory path: %s\n", Dir_path);
	else{								// current path print
		strcpy(temp_path, cwd);
		printf("Directory path: %s\n", temp_path);
	}
	for(i=0 ; i<cnt ; i++)						// total counting
	{
		if(struct_arr[i].hidden_flag==0)	
			total=total+struct_arr[i].st_blocks;
	}
	
	if(hflag==1){	 //h option of total set			
		if(total>=G){				// If G,
			total=total/2;	
			value=total/G;
			rest=total%G;
			for(j=0 ; rest>10 ; j++)
				rest=rest/10;
				printf("%d.%dG	\n", value, rest);
		}
		else if(total<=G && total>=K){		// If K,
			total=total/2;
			value=total/K;
			rest=total%K;
			for(j=0 ; rest>10 ; j++)
				rest=rest/10;
				printf("%d.%dK	\n", value, rest);
		}
		else if(total<=K && total>=M){		// If M,
			total=total/2;
			value=total/M;
			rest=total%M;
			for(j=0 ; rest>10 ; j++)
				rest=rest/10;
			printf("%d.%dM	\n", value, rest);		
		}
		else{
			printf("total: %d\n", total/2);	// total print
		}
					
	}	
	else
		printf("total: %d\n", total/2);		// total print
		
	if(wild_flag==0){	// No wild card
		for(i=0 ; i<cnt ; i++)
		{
			if(struct_arr[i].hidden_flag==0)	// except hidden file
			{	
				if(sflag==1)			// s-flag
					printf("%3ld %3c", struct_arr[i].st_blocks/2, 0);
				////////////////////////////////////////////////////Mecro using, permission print
				if(S_ISDIR(struct_arr[i].st_mode)) printf("d");		// directory, "d"
	   			else if(S_ISLNK(struct_arr[i].st_mode)) printf("l");	// link "l"
	   			else printf("-");					// else regular, "-"
				////////////////////////////////////////////////////
	   			if((struct_arr[i].st_mode&S_IREAD)!=0) printf("r");	// user read
	   			else printf("-");
	   			if((struct_arr[i].st_mode&S_IWRITE)!=0) printf("w");	// user write
	   			else printf("-");
	   			if((struct_arr[i].st_mode&S_IEXEC)!=0)printf("x");	// user exe
	   			else printf("-");
				////////////////////////////////////////////////////
	  			 if((struct_arr[i].st_mode&S_IREAD>>3)!=0) printf("r");	// gr read
	   			else printf("-");
	   			if((struct_arr[i].st_mode&S_IWRITE>>3)!=0) printf("w");	// gr write
	   			else printf("-");
	   			if((struct_arr[i].st_mode&S_IEXEC>>3)!=0)printf("x");	// gr exe
	   			else printf("-");
				////////////////////////////////////////////////////
	  			 if((struct_arr[i].st_mode&S_IREAD>>6)!=0) printf("r");	// ot read
	  			 else printf("-");
	   			if((struct_arr[i].st_mode&S_IWRITE>>6)!=0) printf("w"); // ot write
	   			else printf("-");
	   			if((struct_arr[i].st_mode&S_IEXEC>>6)!=0)printf("x ");	// ot exe
	  			else printf("- ");
				////////////////////////////////////////////////////
				printf("%3d	", struct_arr[i].st_nlink);		// each struct_arr file data print
				printf("%-5s", struct_arr[i].us_name);
				printf("%-10s", struct_arr[i].gr_name);
				if(hflag==1){	 //h option
							
					if(struct_arr[i].st_size>=G){			// If G
						value=struct_arr[i].st_size/G;
						rest=struct_arr[i].st_size%G;
						for(j=0 ; rest>10 ; j++)
							rest=rest/10;
						printf("%d.%dG	", value, rest);
					}
					else if(struct_arr[i].st_size<=G && struct_arr[i].st_size>=K){
						value=struct_arr[i].st_size/K;		// If K
						rest=struct_arr[i].st_size%K;
						for(j=0 ; rest>10 ; j++)
							rest=rest/10;
						printf("%d.%dK	", value, rest);
					}
					else if(struct_arr[i].st_size<=K && struct_arr[i].st_size>=M){
						value=struct_arr[i].st_size/M;		// If M
						rest=struct_arr[i].st_size%M;
						for(j=0 ; rest>10 ; j++)
							rest=rest/10;
						printf("%d.%dM	", value, rest);	// print
					}
						
					else{						// low number
						printf("%ld	", struct_arr[i].st_size);
					}
					
				}	
				else{	// no option
					printf("%10ld", struct_arr[i].st_size);
				}
				printf("%5d", struct_arr[i].tm_mon+1);
				printf("%5d	", struct_arr[i].tm_mday);
				////////////////////////////////////////////////////
				if(struct_arr[i].tm_hour<10)				// time struct_arr time data print
					printf("0%d:", struct_arr[i].tm_hour);
				else	
					printf("%d:", struct_arr[i].tm_hour);	
				if(struct_arr[i].tm_min<10)
					printf("0%d ", struct_arr[i].tm_min);
				else
					printf("%d ", struct_arr[i].tm_min);
				printf("%s\n", struct_arr[i].file_name);
				////////////////////////////////////////////////////	
			}
		}
	}
	else{		// wild card
		for(i=0 ; i<cnt ; i++)
		{
			if(struct_arr[i].hidden_flag==0)	// except hidden file
			{	
				if(struct_arr[i].w_flag==1){	// if having wildcard flag
					
					if(struct_arr[i].d_flag==0){	// if no having directory flag
						if(sflag==1)	// s flag
							printf("%3ld %3c", struct_arr[i].st_blocks/2, 0);
						////////////////////////////////////////////////////Mecro using, permission print
						if(S_ISDIR(struct_arr[i].st_mode)) printf("d");		// directory, "d"
	   					else if(S_ISLNK(struct_arr[i].st_mode)) printf("l");	// link "l"
	   					else printf("-");					// else regular, "-"
						////////////////////////////////////////////////////
			   			if((struct_arr[i].st_mode&S_IREAD)!=0) printf("r");	// user read
			   			else printf("-");
			   			if((struct_arr[i].st_mode&S_IWRITE)!=0) printf("w");	// user write
			   			else printf("-");
			   			if((struct_arr[i].st_mode&S_IEXEC)!=0)printf("x");	// user exe
			   			else printf("-");
						////////////////////////////////////////////////////
			  			 if((struct_arr[i].st_mode&S_IREAD>>3)!=0) printf("r");	// gr read
			   			else printf("-");
			   			if((struct_arr[i].st_mode&S_IWRITE>>3)!=0) printf("w");	// gr write
			   			else printf("-");
			   			if((struct_arr[i].st_mode&S_IEXEC>>3)!=0)printf("x");	// gr exe
			   			else printf("-");
						////////////////////////////////////////////////////
			  			 if((struct_arr[i].st_mode&S_IREAD>>6)!=0) printf("r");	// ot read
			  			 else printf("-");
			   			if((struct_arr[i].st_mode&S_IWRITE>>6)!=0) printf("w"); // ot write
			   			else printf("-");
			   			if((struct_arr[i].st_mode&S_IEXEC>>6)!=0)printf("x ");	// ot exe
			  			else printf("- ");
						////////////////////////////////////////////////////
						printf("%3d	", struct_arr[i].st_nlink);		// each struct_arr file data print
						printf("%-5s", struct_arr[i].us_name);
						printf("%-10s", struct_arr[i].gr_name);
						if(hflag==1){	 //h option
							
							if(struct_arr[i].st_size>=G){			// If G
								value=struct_arr[i].st_size/G;
								rest=struct_arr[i].st_size%G;
								for(j=0 ; rest>10 ; j++)
									rest=rest/10;
								printf("%d.%dG	", value, rest);
							}
							else if(struct_arr[i].st_size<=G && struct_arr[i].st_size>=K){
								value=struct_arr[i].st_size/K;		// If K
								rest=struct_arr[i].st_size%K;
								for(j=0 ; rest>10 ; j++)
									rest=rest/10;
								printf("%d.%dK	", value, rest);
							}
							else if(struct_arr[i].st_size<=K && struct_arr[i].st_size>=M){
								value=struct_arr[i].st_size/M;		// If M
								rest=struct_arr[i].st_size%M;
								for(j=0 ; rest>10 ; j++)
									rest=rest/10;
								printf("%d.%dM	", value, rest);
							}
						
							else{						// low num
								printf("%ld	", struct_arr[i].st_size);
							}
					
						}	
						else{	// no h option
							printf("%10ld", struct_arr[i].st_size);
						}
						printf("%5d", struct_arr[i].tm_mon+1);
						printf("%5d	", struct_arr[i].tm_mday);
						////////////////////////////////////////////////////
						if(struct_arr[i].tm_hour<10)				// time struct_arr time data print
							printf("0%d:", struct_arr[i].tm_hour);
						else	
							printf("%d:", struct_arr[i].tm_hour);	
						if(struct_arr[i].tm_min<10)
							printf("0%d ", struct_arr[i].tm_min);
						else{
							printf("%d ", struct_arr[i].tm_min);
						}
						printf("%s\n", struct_arr[i].file_name);
				////////////////////////////////////////////////////
					}
				}	
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
	char* cwd_buf;						// for getcwd systemcall
	char* cwd;
	int w_cnt=0;						// wildcard file or dir count
	if(wild_path==NULL){					// path decide
		cwd = getcwd(cwd_buf, 1024);
		wild_path=cwd;					// current path
	} 

	if( (strcmp(al_flag, "00")==0) || (strcmp(al_flag, "10")==0) ){		// -a = 0 , -l = 0 || -a = 1, -l = 0
		Dir_path=wild_path;				// path modify
		printf("\nDirectory path: %s\n", wild_path);	
		for(i=0 ; i<cnt; i++){						// when d_flag=0, print wildcard file
			if(struct_arr[i].hidden_flag!=1 && struct_arr[i].w_flag==1 && struct_arr[i].d_flag==0)
				printf("%s\n",struct_arr[i].file_name);	
		}
	}
	else if((strcmp(al_flag, "01")==0) || (strcmp(al_flag, "11")==0) ){ 	// -a = 0, -l = 1 // -a = 1, -l = 1
		Dir_path=wild_path;				// path modify
		l_print(struct_arr, cnt);			// l_print call
	}

	strcpy(save_path, wild_path);	// save_path re-saving
	for(i=0 ; i<cnt; i++)							
	{									// when d_flag=1, print wildcard file of dir
		if(struct_arr[i].hidden_flag==0 && struct_arr[i].w_flag==1 && struct_arr[i].d_flag==1){
			strcat(save_path, "/");					// string cat '/'
			strcat(save_path, struct_arr[i].file_name);		// string cat dir name
			dirp=opendir(save_path);				// open!!
			if(dirp==NULL)
				break; 
			printf("\nDirectory path: %s\n", struct_arr[i].file_name);	// no having dir
			while((dir=readdir(dirp))!=NULL){
				w_cnt++;					// w dir count
			}
			closedir(dirp);
			Dir_path=save_path;
			struct_arr2=(struct data*)malloc(sizeof(struct data)*w_cnt);// struct array allcation 
			memset(struct_arr2, 0, sizeof(struct data)*w_cnt);
			for(j=0 ; j<w_cnt ; j++)				// wd_flag.. it dir of content for print 
				struct_arr2[j].wd_flag=1;
			save(struct_arr2, save_path);				// data inserting
			array(struct_arr2, w_cnt);				// struct sorting

			if( (strcmp(al_flag, "00")==0) || (strcmp(al_flag, "01")==0) ){
				del_hidden(struct_arr2, w_cnt);			// delete hidden
			}
			if( (strcmp(al_flag, "00")==0)){	// no option
				for(j=0 ; j<w_cnt ; j++){
					if(struct_arr2[j].hidden_flag==0)	// no having hidden
						printf("%s\n",struct_arr2[j].file_name);
				}
			}
			else if( (strcmp(al_flag, "10")==0) ){	// -a option
				for(j=0 ; j<w_cnt ; j++)
					printf("%s\n",struct_arr2[j].file_name);
			}					// -l or -al option
			else if( (strcmp(al_flag, "01")==0) || (strcmp(al_flag, "11")==0) )
				l_print(struct_arr2, w_cnt);
			
			printf("\n");
			free(struct_arr2);		//deallocation
			w_cnt=0;			// initializing for re-use
			strcpy(save_path, wild_path);
			wild_flag=0;
		}
		
	}
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
