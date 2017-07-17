#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char* argv[])
	{
	int i, mirror_port, sockfd;
	char mirror_addr[20], content_servers[1024];
	
	struct sockaddr_in server; 
	struct hostent *rem;
	
	if (argc != 7)
		{
		printf("Wrong input arguments! \n");
		return -1;		
		}
		
	// Parse arguments from command line
	for (i = 1 ; i < (argc-1) ; i++)
		{
		if (!strcmp(argv[i], "-n"))
			{
			stpcpy(mirror_addr, argv[++i]);
			continue;
			}
		if (!strcmp(argv[i], "-p"))
			{
			mirror_port = atoi(argv[++i]);
			continue;
			}
		if (!strcmp(argv[i], "-s"))
			{
			stpcpy(content_servers, argv[++i]);
			continue;
			}
		printf("Wrong input arguments! \n");
		return -1;
		}
	
	
	// Create socket to connect with MirrorServer 
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
		perror("socket");
		exit(1);
		}
	
	if ((rem = gethostbyname(mirror_addr)) == NULL ) 
		{
		perror("gethostbyname"); 
		exit(2);
		}
		
	server.sin_family = AF_INET;
	memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
	server.sin_port = htons(mirror_port);
	
		
	// Establish connection to MirrorServer 
	if (connect(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0)
		{
		perror("connect");
		exit(13);
		}
		
	
	printf("Connecting to %s on port %d\n", inet_ntoa(server.sin_addr), mirror_port);	
		
	// Send Content Server's info to Mirror Server
	if (write(sockfd, content_servers, sizeof(content_servers)) < 0)
		{	
		perror("write");
		exit(7);
		}
				
	// Read statistics from Mirror Server
	if (read(sockfd, content_servers, sizeof(content_servers)) < 0)
		{	
		perror("read");
		exit(6);
		}

	printf("------------ Statistics ------------\n");	
	printf("\n%s", content_servers);	
	
	printf("\nMirror initiator exiting...\n");
				
	close(sockfd);
	
	return 0;
	}
	