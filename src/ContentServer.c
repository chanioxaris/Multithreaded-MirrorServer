#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


#define MAX_QUEUE_CONNECTIONS 5

#include "functions.h"

int main(int argc, char* argv[]) {
	int i, port, sockfd, client_socket, remain_bytes;
	char dir_name[28], buf[1024];
	char *token;

	struct sockaddr_in server, client;
	struct stat file_stat;

	size_t client_len = sizeof(client), bytes;

	FILE *fd;

	if (argc != 5) {
		printf("Wrong input arguments! \n");
		return -1;
	}

	// Parse arguments from command line
	for (i = 1 ; i < (argc-1) ; i++) {
		if (!strcmp(argv[i], "-p")) {
			port = atoi(argv[++i]);
			continue;
		}
		if (!strcmp(argv[i], "-d")) {
			stpcpy(dir_name, argv[++i]);
			continue;
		}

		printf("Wrong input arguments! \n");
		return -1;
	}

	// Create socket to connect with MirrorInitiator
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(port);

	if (bind(sockfd, (struct sockaddr *) &server, sizeof(server))) {
		perror("bind");
		exit(3);
	}

	if (listen(sockfd, MAX_QUEUE_CONNECTIONS) < 0) {
		perror("listen");
		exit(4);
	}

	while(1) {
		// Accept connection from Mirror managers and workers threads
		if ((client_socket = accept(sockfd, (struct sockaddr *) &client, &client_len)) < 0) {
			perror("accept");
			exit(5);
		}

		// Read command
		if (read(client_socket, buf, sizeof(buf)) < 0) {
			perror("read");
			exit(6);
		}

		token = strtok(buf, " ");

		if (!strcmp(token, "LIST")) {
			memset(buf, 0, sizeof(buf));

			list_dir(dir_name, buf);

			if (write(client_socket, buf, sizeof(buf)) < 0) {
				perror("write");
				exit(7);
			}
		}
		else if (!strcmp(token, "FETCH")) {
			token = strtok(NULL, " ");

			// Get stats of requested file
			if (stat(token, &file_stat) < 0) {
                perror("stat");
				exit(15);
			}

			remain_bytes = file_stat.st_size;

			sprintf(buf, "%d", remain_bytes);

			// Send requested file size
			if (write(client_socket, buf, sizeof(buf)) < 0) {
				perror("write");
				exit(7);
			}

			if ((fd = fopen(token, "rb")) < 0) {
				perror("fopen");
				exit(14);
			}

			token = strtok(NULL, " ");

			// Append delay before sending data
			sleep(atoi(token));

			// Sending file data
			while ((bytes = fread(buf, 1, sizeof(buf), fd)) > 0)
				send(client_socket, buf, bytes, 0);

			fclose(fd);
		}
		else if (!strcmp(token, "EXIT")) {
			close(client_socket);
			break;
		}

		close(client_socket);
	}

	close(sockfd);

	return 0;
}
