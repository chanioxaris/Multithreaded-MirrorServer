#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h> 

#include "functions.h"


// Function to initialize the shared queue
Queue *queue_init()
	{
	int i;	
	Queue *queue;	
		
	if ((queue = malloc(sizeof(Queue))) == NULL)
		{
		perror("malloc");	
		exit(8);	
		}
		
	queue->elements = 0;
	queue->front = -1;
	queue->rear = -1;
	
	
	if ((queue->buffer = malloc(BUFFER_SIZE * sizeof(char *))) == NULL)
		{
		perror("malloc");	
		exit(8);	
		}
	
	for (i = 0 ; i < BUFFER_SIZE ; i++)
		{
		if ((queue->buffer[i] = malloc(BUFFER_STR_LEN * sizeof(char))) == NULL)
			{
			perror("malloc");	
			exit(8);	
			}
		}	
	return queue;
	}

	
// Function to find occurencies of delimiter in string
int delimiter_occurrences(char *str, int del)
	{
	int i, count = 1;
    
	for (i=0 ; str[i] ; i++)
		{
		if (str[i] == del)
			count++;			
		}
    
    return count;		
	}

	
// Function to find contents of directory using recursion	
void list_dir(char *path, char *structure)
	{
	DIR *dir;
	char d_path[255]; 
	struct dirent *entry; 
   
	if((dir = opendir(path)) == NULL) 
		return; // if was not able return	

		
	while ((entry = readdir(dir)) != NULL) 
		{
		sprintf(d_path, "%s/%s", path, entry->d_name);	
			
		if(entry-> d_type != DT_DIR) 
			sprintf(structure, "%s%s:f,", structure, d_path);
		else if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) 
			{			
			sprintf(structure, "%s%s:d,", structure, d_path);
			
			list_dir(d_path, structure); 			
			}
		}
	closedir(dir); 
	}		

	
// Function to dynamic allocate memory of 2d array of strings
char ***content_server_info_init(int number_managers)
	{
	int i, j;
	char ***content_server_info;
		
	/* Allocate array with Content Servers Info */	
	if ((content_server_info = malloc(number_managers * sizeof(char **))) == NULL)
		{
		perror("malloc");	
		exit(8);				
		}

		
	for (i = 0 ; i < number_managers ; i++)
		{
		if ((content_server_info[i] = malloc(4 * sizeof(char *))) == NULL)
			{
			perror("malloc");	
			exit(8);	
			}
		}
	
	for (i = 0 ; i < number_managers ; i++)
		{
		for (j = 0 ; j < 4 ; j++)	
			{
			if ((content_server_info[i][j] = malloc(20 * sizeof(char))) == NULL)
				{
				perror("malloc");	
				exit(8);	
				}
			}
		}

	return content_server_info;	
	}