#ifndef _THREADS_
#define _THREADS_

#include <pthread.h>

pthread_mutex_t mutex_queue, mutex_glb;
pthread_cond_t empty, full, allDone;

extern int numDevicesDone;
extern int directories;
extern int files;
extern int bytes_transfered;

void *MirrorWorkers(void *);

void *MirrorManagers(void *);

#endif