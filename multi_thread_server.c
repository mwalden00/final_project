#include "chat_funcs.h"

int main()
{
  int listen_fd = server_setup();

  // Create + initialize the chat data struct
  ChatData data;
  data.num_c = 0;
  data.listen_socket = listen_fd;
  data.queue = queue();
  data.c_sockets_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(data.c_sockets_mutex, NULL);

  // Create + initialize the chat acception thread
  // Set accept_client() as its function and the chat data as
  // its arguments. It will coninuously accept clients as the server runs
  pthread_t accept_t;
  pthread_create(&accept_t, NULL, (void *)&accept_client, (void *)&data);
  printf("[CLIENT ACCEPTION THREAD CREATED]\n");

  // Set up the chat fd_set for the messaging system
  FD_ZERO(&(data.s_read_fds));
  FD_SET(listen_fd, &(data.s_read_fds));

  // Create + initialize the messaging thread
  // Set messenger() as its function and the chat data as
  // its arguments. It will continually handle messaging as the server runs
  pthread_t message_t;
  pthread_create(&message_t, NULL, (void *)&messenger, (void *)&data);
  printf("[MESSAGE THREAD CREATED]\n");

  pthread_join(accept_t, NULL);
  pthread_join(message_t, NULL);
  destroy_q(data.queue);
  pthread_mutex_destroy(data.c_sockets_mutex);
  free(data.c_sockets_mutex);
}