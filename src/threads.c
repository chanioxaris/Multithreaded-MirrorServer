#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#include "threads.h"
#include "functions.h"


int numDevicesDone;

int directories;
int files;
int bytes_transfered;

Queue *shared_queue;

void *MirrorWorkers(void *arg)
	{
	int content_servers = (int) arg, sockfd;	
	char buf[1024], addr[24], port[12], path[50], new_dir[64], requested[12], type[2], delay[5];		

	char *token;
	
	struct sockaddr_in server; 
	struct hostent *rem;	
	
	size_t bytes;

	FILE *new_file; 
				
	while(1)	
		{			
		if (numDevicesDone == content_servers)
			break;	

						
		pthread_mutex_lock(&mutex_queue);	
		
		// Sleep if queue is empty
		while(shared_queue->elements == 0)
			pthread_cond_wait(&empty, &mutex_queue);	

			
		// Remove from shared_queue
		shared_queue->front = (shared_queue->front + 1) % BUFFER_SIZE;
		shared_queue->elements--;
		stpcpy(buf, shared_queue->buffer[shared_queue->front]);
		
		token = strtok(buf, ",");
		stpcpy(type, token);
		
		// If no more paths, then increase global variable numDevicesDone
		if(!strcmp(type, "e"))
			{
			numDevicesDone++;
			
			if (numDevicesDone == content_servers)
				pthread_cond_signal(&allDone);	
			}
				
		pthread_mutex_unlock(&mutex_queue);
		pthread_cond_signal(&full);
	
		token = strtok(NULL, ",");
		stpcpy(requested, token);	
							
		token = strtok(NULL, ",");
		stpcpy(path, token);
								
		token = strtok(NULL, ",");
		stpcpy(addr, token);
		
		token = strtok(NULL, ",");
		stpcpy(port, token);
		
		token = strtok(NULL, ",");
		stpcpy(delay, token);
		
		// If directory then create it		
		if (!strcmp(type, "d"))
			{									
			sprintf(new_dir, "%s_%s/%s", addr, port, strstr(path, requested));

			if (mkdir(new_dir, 0700) == -1) 
				{
				perror("mkdir");
				exit(11);
				}
			
			// Global variable for statistics
			pthread_mutex_lock(&mutex_glb);	
			
			directories++;
			
			pthread_mutex_unlock(&mutex_glb);			
			}	
		// If file then get it from Content Server
		else if (!strcmp(type, "f"))
			{
			// Create socket to connect with ContentServer
			if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
				{
				perror("socket");
				exit(1);
				}
			
			if ((rem = gethostbyname(addr)) == NULL ) 
				{
				perror("gethostbyname"); 
				exit(2);
				}

			server.sin_family = AF_INET;
			memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
			server.sin_port = htons(atoi(port));

			// Establish connection to ContentServer
			if (connect(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0)
				{
				perror("connect");
				exit(13);
				}

			// Create message for FETCH command
			sprintf(buf, "FETCH %s %s", path, delay);	
				
			if (write(sockfd, buf, sizeof(buf)) < 0)
				{	
				perror("write");
				exit(7);
				}
			
			// Read total size of file
			if (read(sockfd, buf, sizeof(buf)) < 0)
				{	
				perror("read");
				exit(6);
				}
			
			// Global variables for statistics
			pthread_mutex_lock(&mutex_glb);
			
			files++;
			bytes_transfered += atoi(buf);		
			
			pthread_mutex_unlock(&mutex_glb);
			
			
			sprintf(new_dir, "%s_%s/%s", addr, port, strstr(path, requested));
			
			// Create new file
			if ((new_file = fopen(new_dir, "w")) == NULL)
				{
				perror("fopen");
				exit(14);	
				}
	
			// Get data of file using loop
			while (((bytes = recv(sockfd, buf, sizeof(buf), 0)) > 0))
                fwrite(buf, sizeof(char), bytes, new_file);

			fclose(new_file);					
			close(sockfd);
			}			
		}	

	pthread_exit(NULL);
	}


	
void *MirrorManagers(void *arg)
	{		
	int i, sockfd, err;
	char buf[1024], path[32];
	char buf_queue[BUFFER_STR_LEN];
	
	char *token_list, *tmp_token_list, *token_path, *tmp_token_path;
	char **info = arg;	
		
	struct sockaddr_in server; 
	struct hostent *rem;	
	
	// Create socket to connect with Content Server 
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
		perror("socket");
		exit(1);
		}
	
	if ((rem = gethostbyname(info[0])) == NULL ) 
		{
		perror("gethostbyname"); 
		exit(2);
		}

	server.sin_family = AF_INET;
	memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
	server.sin_port = htons(atoi(info[1]));
	
	// Establish connection to Content Server
	if ((err = connect(sockfd, (struct sockaddr *) &server, sizeof(server))) < 0)
		{
		perror("connect");
		exit(13);	
		}

	// Create message for LIST command 
	sprintf(buf, "LIST");
	
	if (write(sockfd, buf, sizeof(buf)) < 0)
		{	
		perror("write");
		exit(7);
		}
	
	// Get the names of all directed available for sharing
	if (read(sockfd, buf, sizeof(buf)) < 0)
		{	
		perror("read");
		exit(6);
		}
	
	token_list = strtok_r(buf, ",", &tmp_token_list);

	// Fill shared buffer with data
	while(token_list != NULL)	
		{	
		// Filter paths and insert on buffer only the needed
		if(strstr(token_list, info[2]) != NULL) 
			{
			pthread_mutex_lock(&mutex_queue);	

			// Sleep if buffer is full
			while(shared_queue->elements == BUFFER_SIZE)
				pthread_cond_wait(&full, &mutex_queue);	
			

			token_path = strtok_r(token_list, ":", &tmp_token_path);
			stpcpy(path, token_path);
			
			token_path = strtok_r(NULL, ":", &tmp_token_path);

			sprintf(buf_queue, "%s,%s,%s,%s,%s,%s", token_path, info[2], path, info[0], info[1], info[3]);
											
			// Insert into shared_queue
			shared_queue->rear = (shared_queue->rear + 1) % BUFFER_SIZE;
			shared_queue->elements++;
			stpcpy(shared_queue->buffer[shared_queue->rear], buf_queue);
			
			pthread_mutex_unlock(&mutex_queue);
			pthread_cond_signal(&empty);						
			}	
		token_list = strtok_r(NULL, ",", &tmp_token_list);	
		}
		
	pthread_mutex_lock(&mutex_queue);	
		
	while(shared_queue->elements == BUFFER_SIZE)
		pthread_cond_wait(&full, &mutex_queue);	

	
	sprintf(buf_queue, "e,%s,END,%s,%s,%s", info[2], info[0], info[1], info[3]);
	
	// Insert END MESSAGE into queue
	shared_queue->rear = (shared_queue->rear + 1) % BUFFER_SIZE;
	shared_queue->elements++;
	stpcpy(shared_queue->buffer[shared_queue->rear], buf_queue);
		
	pthread_mutex_unlock(&mutex_queue);
	pthread_cond_signal(&empty);
	
	
	close(sockfd);

	pthread_exit(NULL);
	}