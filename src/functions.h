#ifndef _FUNCTIONS_
#define _FUNCTIONS_

#define BUFFER_SIZE 10
#define BUFFER_STR_LEN 128


typedef struct Queue
	{
	int elements;
	int front;
	int rear;
	char **buffer;
	}Queue;

	
extern Queue *shared_queue;

Queue *queue_init();

int delimiter_occurrences(char *, int);

void list_dir(char *, char *);

char ***content_server_info_init(int);

#endif