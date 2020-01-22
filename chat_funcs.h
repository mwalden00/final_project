#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <pthread.h>
#include "networking.h"

/*
   Basic implementation of queue structure
   buffer acts as a list of strings on the queue
   head, tail are number representations of the head and tail locations
   full, empty are basically booleans for is full / is empty
   mutex is a way for thread functions to lock on to the queue itself
   notFull, notEmpty are conditionals for threading functions
*/
typedef struct {
    char *buffer[BUFFER_SIZE];
    int head, tail, full, empty;
    pthread_mutex_t * mutex;
    pthread_cond_t * notFull, * notEmpty;
} Queue;

/*
   Conatains data on the chat itself
   s_read_fds is a file set to be used on select()
   listen_socket is the listen_socket
   c_sockets is a list of all client socket fds
   num_c is the number of clients
   c_socketsMutex is a mutex for threading functions
   queue is a queue for messages (see above)
*/
typedef struct {
    fd_set s_read_fds;
    int listen_socket;
    int c_sockets[BUFFER_SIZE];
    int num_c;
    int mafia;
    int mafia_members[FD_SETSIZE];
    pthread_mutex_t *c_sockets_mutex;
    Queue *queue;
} ChatData;

/*
   Handles individual clients
   data contains the chat data struct;
   it will be used to communicate between clients
   c_socket is the client socket fd
*/
typedef struct {
    ChatData *data;
    int c_socket;
} ClientHandler;

Queue * queue();
void destroy_q(Queue * q);
void push(Queue * q, char * str);
char * pop(Queue * q);
void accept_client(void * data);
void * client(void * data);
void messenger(void * data);
void read_input(void * data);
