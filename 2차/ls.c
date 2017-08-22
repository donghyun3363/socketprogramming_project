#include <stdio.h>
#include <dirent.h>		// dirent structure headerfile
#include <string.h>
#include <stdlib.h>		// for 2 dynamic allocation of address values
int main()
{
	DIR *dirp;		// directory stream structure
	struct dirent *dir;	// directory stream entry structure
	char** save;		// double pointer array for 2D
	char temp[200]={0,};	// saving array
	int cnt=0;		// counter
	int i=0, j=0, k=0, t=0, z=0, f=0;	// index for for statement and so on
	char min[200]={0,};	// minimum array
	//////////////////////////////////////////////////
	dirp=opendir(".");			// it counts directory entry files in dirp
	while((dir=readdir(dirp))!=NULL)
		cnt++;
	closedir(dirp);
	//////////////////////////////////////////////////
	save = (char**)malloc(sizeof(char*)*cnt);
	for(i=0 ; i<cnt ; i++)			// 2 dynamic allocation and initializing for array
	{
		save[i] = (char*)malloc(sizeof(char)*200);
		memset(save[i], 0, sizeof(char)*200);
	}		
	i=0;					// index initializing
	//////////////////////////////////////////////////
	dirp=opendir(".");			// After file reads on directory stream, saving to save array each others
	for(dir = readdir(dirp); dir; dir = readdir(dirp))
	{
		strcpy(save[i], dir->d_name);
		i++;
	}
	closedir(dirp);
	//////////////////////////////////////////////////
	while(t<cnt)				// while statement from 0 to cnt
	{
		//////////////////////////////////////////////////
		for(z=0 ; z<cnt ; z++) 		// Searching to minimum values of array
		{
			if(save[z]==NULL)
				continue;
			
			else
			{
				strcpy(min, save[z]);
				break;
			}
		}
		//////////////////////////////////////////////////
		for(i=0 ; i<cnt ; i++)		
		{	
			if(save[i]!=NULL)			// when not null,
			{
				//////////////////////////////////////////////////
				if(((save[i])[0]=='.')&&((save[i])[1]!='.')&&((save[i])[1]!=0))
				{					// if the data is hidden file, 
					for(j=0 ; save[i][j]!=0 ; j++)	// resaving without '.' 
						temp[j]=(save[i])[j+1];
								
					for(f=0 ; temp[f]!=0 ; f++)	// solving about captical character
					{
						if(temp[f]>=65 && temp[f]<=90)			
							temp[f]+=32;
					}
				}
				//////////////////////////////////////////////////
				else
				{	
					strcpy(temp, save[i]);		// if the data isn't hidden file, 
					for(f=0; temp[f]!=0; f++)
					{				// solving about captical character
						if(temp[f]>=65 && temp[f]<=90)			
							temp[f]+=32;
					}
				}
				//////////////////////////////////////////////////
				if(strcmp(save[i],min)==0)		// it blocks to change min when it's about hidden or captical- 
					strcpy(min,temp);
				//////////////////////////////////////////////////
				if(strcmp(min, temp)>=0)		// changing min values
				{
					strcpy(min, temp);
					k=i;				// saving i index
				}		
				
			}
		}
		printf("%s\n", save[k]);				// printing minimum values of array
		save[k]=NULL;						// changing to NULL
		t++;							// inclease while statement index 
		memset(temp, 0, sizeof(char)*200);			// initializing temp array
	}
	free(save);							// 2D array cancellation.
	return 0;
}
