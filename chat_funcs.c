#include "chat_funcs.h"

Queue * queue() {
  Queue * q = (Queue * )malloc(sizeof(Queue));
  q->empty = 1;
  q->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(q->mutex, NULL);
  q->notFull = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
  q->notEmpty = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
  pthread_cond_init(q->notFull, NULL);
  pthread_cond_init(q->notEmpty, NULL);
  q->head = q->tail = q->full = 0;
  return q;
}

void destroy_q(Queue * q) {
  pthread_mutex_destroy(q->mutex);
  pthread_cond_destroy(q->notFull);
  pthread_cond_destroy(q->notEmpty);
  free(q->mutex);
  free(q->notFull);
  free(q->notEmpty);
  free(q);
}

void push(Queue * q, char * str) {
  q->buffer[q->tail] = str;
  q->tail++;
  if (q->tail == BUFFER_SIZE) q->tail = 0;
  if (q->tail == q->head) q->full = 1;
  q->empty = 0;
}

char * pop(Queue * q) {
  char * ret = q->buffer[q->head];
  q->head++;
  if (q->head == BUFFER_SIZE) q->head = 0;
  if (q->tail == q->head) q->empty = 1;
  q->full = 0;
  return ret;
}

void disconnect(ChatData * data, int c_socket) {
  pthread_mutex_lock(data->c_sockets_mutex);
  int i;
  for (i = 0; i < BUFFER_SIZE; i++) {
    if (data->c_sockets[i] == c_socket) {
      data->c_sockets[i] = 0;
      close(c_socket);
      data->num_c--;
      break;
    }
  }
  pthread_mutex_unlock(data->c_sockets_mutex);
}

void accept_client(void * data) {
  ChatData * c_data = (ChatData *)data;
  while (1) {
    // Make connection w/ client
    int c_socket = server_connect(c_data->listen_socket);
    // Lock c_sockets mutex to this function; this allows for all actions
    // to be synchronized in the server.
    pthread_mutex_lock(c_data->c_sockets_mutex);
    if (c_data->num_c < BUFFER_SIZE) {
      // Add c_socket to socket fd_set
      // It will take an empty slot or a slot of a disconnected client
      int i;
      for (i = 0; i < BUFFER_SIZE; i++) {
        if (!FD_ISSET(c_data->c_sockets[i], &(c_data->s_read_fds))) {
          c_data->c_sockets[i] = c_socket;
          break;
        }
      }
      FD_SET(c_socket, &(c_data->s_read_fds));
      // Create / initialize new client thread.
      // This thread handles all messages by the single client.
      ClientHandler c_handle;
      c_handle.c_socket = c_socket;
      c_handle.data = c_data;
      pthread_t client_t;
      if (pthread_create(&client_t, NULL, (void *)&client, (void *)&c_handle) == 0) {
        c_data->num_c++;
        printf("\r[CLIENT %d HAS JOINED]\n>> ", c_socket);
        fflush(stdout);
      } else close(c_socket);
    }
    pthread_mutex_unlock(c_data->c_sockets_mutex);
  }
}

void * client(void * data) {
  ClientHandler * client_data = (ClientHandler *) data;
  ChatData * c_data = (ChatData *) client_data->data;
  Queue * queue = c_data->queue;

  int c_socket = client_data->c_socket;
  while(queue->full) pthread_cond_wait(queue->notFull, queue->mutex);
  pthread_mutex_lock(queue->mutex);
  push(queue, "[SERVER ANNOUNCMENT] -> Client has entered the server");
  pthread_mutex_unlock(queue->mutex);
  pthread_cond_signal(queue->notEmpty);
  while (1) {
    char buffer[BUFFER_SIZE];
    read(c_socket, buffer, sizeof(buffer));
    while(queue->full) pthread_cond_wait(queue->notFull, queue->mutex);
    // Lock queue mutex to function, push message onto the queue.
    if (strcmp(buffer, "/exit\n") == 0 || strcmp(buffer, "/exit") == 0) {
      printf("\r[CLIENT %d DISCONNECTED]\n>> ", c_socket);
      fflush(stdout);
      pthread_mutex_lock(queue->mutex);
      push(queue, "[SERVER ANNOUNCMENT] -> Client has disconnected");
      disconnect(c_data, c_socket);
      pthread_mutex_unlock(queue->mutex);
      pthread_cond_signal(queue->notEmpty);
      return NULL;
    }
    pthread_mutex_lock(queue->mutex);
    push(queue, buffer);
    pthread_mutex_unlock(queue->mutex);
    pthread_cond_signal(queue->notEmpty);
  }
}

void messenger(void * data) {
  ChatData * c_data = (ChatData * )data;
  Queue * q = c_data->queue;
  int i;
  int * c_sockets = c_data->c_sockets;
  while (1) {
    pthread_mutex_lock(q->mutex);
    while (q->empty) pthread_cond_wait(q->notEmpty, q->mutex);
    char * msg = pop(q);
    pthread_mutex_unlock(q->mutex);
    pthread_cond_signal(q->notFull);
    if (msg[0] == '[') printf("\r%s\n>> ", msg);
    else printf("\r[SENDING MESSAGE] %s\n>> ", msg);
    fflush(stdout);
    for (i = 0; i < c_data->num_c; i++) {
      int c_socket = c_sockets[i];
      if (FD_ISSET(c_socket, &(c_data->s_read_fds)) && write(c_socket, msg, BUFFER_SIZE - 1) < 0) {
        perror("MESSAGE FAILURE");
      }
    }
  }
}

void read_input(void * data) {
  ChatData * c_data = (ChatData *)data;
  Queue * q = c_data->queue;

  char std_buffer[BUFFER_SIZE];
  while(read(STDIN_FILENO, std_buffer, sizeof(std_buffer))) {
    char msg[BUFFER_SIZE];
    *strchr(std_buffer, '\n') = 0;
    if (strcmp(std_buffer,"/exit") == 0) exit(1);
    sprintf(msg, "[SERVER ANNOUNCMENT] -> %s", std_buffer);
    fflush(stdout);
    while(q->full) pthread_cond_wait(q->notFull, q->mutex);
    pthread_mutex_lock(q->mutex);
    push(q, msg);
    pthread_mutex_unlock(q->mutex);
    pthread_cond_signal(q->notEmpty);
  }
}
