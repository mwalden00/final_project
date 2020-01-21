#include "chat_funcs.h"

ChatData data;
pthread_t accept_t;
pthread_t message_t;
pthread_t stdin_t;

int main()
{
  int listen_fd = server_setup();

  // Create + initialize the chat data struct
  data.num_c = 0;
  data.listen_socket = listen_fd;
  data.queue = queue();
  data.c_sockets_mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(data.c_sockets_mutex, NULL);

  // Set up the chat fd_set for the messaging system
  FD_ZERO(&(data.s_read_fds));
  FD_SET(listen_fd, &(data.s_read_fds));

  // Create + initialize the chat acception thread
  // Set accept_client() as its function and the chat data as
  // its arguments. It will coninuously accept clients as the server runs
  pthread_create(&accept_t, NULL, (void *)&accept_client, (void *)&data);
  printf("\n[CLIENT ACCEPTION THREAD CREATED]\n");

  // Create + initialize the messaging thread
  // Set messenger() as its function and the chat data as
  // its arguments. It will continually handle messaging as the server runs
  pthread_create(&message_t, NULL, (void *)&messenger, (void *)&data);
  printf("[MESSAGE THREAD CREATED]\n");

  // Create + initialize the stdin thread
  // Set read_input() as its function and the chat data as
  // its arguments. It will continually read in from std_in and broadcast
  // server messages.
  pthread_create(&stdin_t, NULL, (void *)&read_input, (void *)&data);
  printf("[STDIN THREAD CREATED]\n\n");

  printf("Type into the server and hit [enter] at any time to do a server announcement.\n");
  printf("Type \"/exit\" in at any time to shut down the server.\n>> ");
  fflush(stdout);
  sleep(1);

  pthread_join(accept_t, NULL);
  pthread_join(message_t, NULL);
  pthread_join(stdin_t, NULL);
  destroy_q(data.queue);
  pthread_mutex_destroy(data.c_sockets_mutex);
  free(data.c_sockets_mutex);
}
