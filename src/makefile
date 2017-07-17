CC = gcc

PROG = MirrorInitiator ContentServer MirrorServer
	
all : $(PROG)


MirrorInitiator: MirrorInitiator.o
	$(CC) $^ -o $@ 

ContentServer: ContentServer.o functions.o 
	$(CC) $^ -o $@ 
	
MirrorServer: MirrorServer.o functions.o threads.o
	$(CC) $^ -o $@ functions.h -lpthread

	
clean :
	rm -f MirrorInitiator.o ContentServer.o functions.o MirrorServer.o threads.o
	