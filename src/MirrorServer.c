#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#include "functions.h"
#include "threads.h"

#define MAX_QUEUE_CONNECTIONS 5


int numDevicesDone;

int directories;
int files;
int bytes_transfered;

Queue *shared_queue;

int main(int argc, char* argv[])
	{
	int i, j, port, number_workers, number_managers, sockfd, initiator_socket, err;
	char path[16], content_dir[32], buf_initiator[1024];	
	char *token_content_server, *tmp_content_server, *token_server_data, *tmp_server_data;	
	char ***content_server_info;

	struct stat sb;	
	struct sockaddr_in server, initiator; 
	size_t initiator_len = sizeof(initiator);
	
	pthread_t *workers, *managers;
	
	if (argc != 7)
		{
		printf("Wrong input arguments! \n");
		return -1;		
		}
		
	// Parse arguments from command line
	for (i = 1 ; i < (argc-1) ; i++)
		{
		if (!strcmp(argv[i], "-p"))
			{
			port = atoi(argv[++i]);
			continue;
			}	
		if (!strcmp(argv[i], "-m"))
			{
			stpcpy(path, argv[++i]);
			continue;
			}
		
		if (!strcmp(argv[i], "-w"))
			{
			number_workers = atoi(argv[++i]);
			continue;
			}
		printf("Wrong input arguments! \n");
		return -1;
		}
	
	// Create directory to save transfered files and directories
	if (mkdir(path, 0700) == -1) 
		{
        perror("mkdir");
        exit(11);
		}
		
	if (chdir(path) == -1) 
		{
        perror("chdir");
        exit(11);
		}		

	// Allocate memory for Mirror workers threads	
	if ((workers = malloc(number_workers * sizeof(pthread_t))) == NULL)
		{
		perror("malloc");	
		exit(8);	
		}
	
	// Create socket to connect with MirrorInitiator 
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
		perror("socket");
		exit(1);
		}
			
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port);
	
	if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)))
		{
		perror("bind");
		exit(3);
		}
	
	
	if (listen(sockfd, MAX_QUEUE_CONNECTIONS) < 0)
		{
		perror("listen");
		exit(4);
		}
	
	printf("Mirror server listening for connections on port %d\n", port);
	
	
	// Accept connection from Mirror Initiators
	if ((initiator_socket = accept(sockfd, (struct sockaddr *) &initiator, &initiator_len)) < 0)
		{
		perror("accept");
		exit(5);
		}
	
	// Read Content Servers info
	if (read(initiator_socket, buf_initiator, sizeof(buf_initiator)) < 0)
		{
		perror("read");
		exit(6);
		}

					
	//	Initialize mutexes and condition variables
	pthread_mutex_init(&mutex_queue, NULL);
	pthread_mutex_init(&mutex_glb, NULL);
	pthread_cond_init(&empty, NULL);
	pthread_cond_init(&full, NULL);
	pthread_cond_init(&allDone, NULL);	
	
	//Initialize shared buffer
	shared_queue = queue_init();		
	
	// Find the number of requested Content Servers
	number_managers = delimiter_occurrences(buf_initiator, ',');
		
	// Initialize global variables
	pthread_mutex_lock(&mutex_glb);
	
	numDevicesDone = 0;
	directories = 0;
	files = 0;
	bytes_transfered = 0;
	
	pthread_mutex_unlock(&mutex_glb);
	

	// Create workers threads 	
	for (i = 0 ; i < number_workers ; i++)
		{
		if ((err = pthread_create(workers + i, NULL, MirrorWorkers, (void *) number_managers)) != 0)
			{
			perror("pthread_create");
			exit(9);
			}
		}	
	

	content_server_info = content_server_info_init(number_managers);

	// Fill array with info for each Content Server 
	token_content_server = strtok_r(buf_initiator, ",", &tmp_content_server);

	for (i = 0 ; i < number_managers ; i++)
		{
		token_server_data = strtok_r(token_content_server, ":", &tmp_server_data);		
		strcpy(content_server_info[i][0], token_server_data);

		token_server_data = strtok_r(NULL, ":", &tmp_server_data);		
		strcpy(content_server_info[i][1], token_server_data);

		token_server_data = strtok_r(NULL, ":", &tmp_server_data);		
		strcpy(content_server_info[i][2], token_server_data);

		token_server_data = strtok_r(NULL, ":", &tmp_server_data);		
		strcpy(content_server_info[i][3], token_server_data);

		
		token_content_server = strtok_r(NULL, ",", &tmp_content_server);
		}


	// Allocate memory for Mirror managers threads
	if ((managers = malloc(number_managers * sizeof(pthread_t))) == NULL)
		{
		perror("malloc");	
		exit(8);	
		}
	
	// Create managers threads
	for (i = 0 ; i < number_managers ; i++)
		{
		if ((err = pthread_create(managers + i, NULL, MirrorManagers, (void *) content_server_info[i])) != 0)
			{
			perror("pthread_create");
			exit(9);
			}
						
		sprintf(content_dir, "%s_%s", content_server_info[i][0], content_server_info[i][1]);
		
		// Check if directory already exists
		if (stat(content_dir, &sb) == 0 && S_ISDIR(sb.st_mode))
			continue;
		
		// Create directory for each Content Server
		if (mkdir(content_dir, 0700) == -1) 
			{
			perror("mkdir");
			exit(11);
			}					
		}
	
	// Sleep until work is done
	pthread_mutex_lock(&mutex_glb);

	while(number_managers != numDevicesDone)
		pthread_cond_wait(&allDone, &mutex_glb);	
	
	pthread_mutex_unlock(&mutex_glb);
	

	// Wait for workers threads to exit 	
	for (i = 0 ; i < number_workers ; i++)
		{
		if ((err = pthread_join(*(workers + i), NULL)) != 0)
			{
			perror("pthread_join");
			exit(10);
			}
		}
	
	// Wait for managers threads to exit 	
	for (i = 0 ; i < number_managers ; i++)
		{
		if ((err = pthread_join(*(managers + i), NULL)) != 0)
			{
			perror("pthread_join");
			exit(10);
			}
		}
	
	sprintf(buf_initiator, "Directories created: %d\nFiles transfered: %d\nBytes transfered: %d\nAverage file size: %d bytes\n", directories, files, bytes_transfered, (bytes_transfered/files));

	// Send statistics back to Mirror Initiator
	if (write(initiator_socket, buf_initiator, sizeof(buf_initiator)) < 0)
		{	
		perror("write");
		exit(7);
		}
	
	// Free array's memory with Content Server info
	for (i = 0 ; i < number_managers ; i++)
		{
		for (j = 0 ; j < 4 ; j++)	
			free(content_server_info[i][j]);	
		}
	
	for (i = 0 ; i < number_managers ; i++)
		free(content_server_info[i]);	
		
	free(content_server_info);
	
	free(managers);
	
	close(initiator_socket);

	pthread_cond_destroy(&allDone);
	pthread_cond_destroy(&full);
	pthread_cond_destroy(&empty);
	pthread_mutex_destroy(&mutex_queue);
	pthread_mutex_destroy(&mutex_glb);
		
		
	// Free shared buffer's memory
	for (i = 0 ; i < BUFFER_SIZE ; i++)
		free(shared_queue->buffer[i]);	
	free(shared_queue->buffer);	
	free(shared_queue);
		
	free(workers);
	
	close(sockfd);
	
	printf("Mirror server exiting...\n");
	
	return 0;
	}