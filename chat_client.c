#include "chat_funcs.h"

int server_socket;

void message(char * to_send, char * buffer, char * name);
void client_sighandle(int sig);

int main(int argc, char * argv[]) {
  char * name;
  fd_set read_fds;
  char buffer[BUFFER_SIZE];
  // connect to server
  if(argc != 3) {
    printf("./client [ip address] [username]\n");
    exit(1);
  }
  printf("\nType messages into the terminal and hit [enter] to send\nSend \"/exit\" to exit\n>> ");
  fflush(stdout);
  server_socket = client_setup(argv[1]);
  // use client_sighandle to handle ^C interrupts
  signal(SIGINT, client_sighandle);
  name = argv[2];
  while(1) {
    FD_ZERO(&read_fds);
    FD_SET(server_socket, &read_fds);
    FD_SET(STDIN_FILENO, &read_fds);
    if (select(server_socket+1, &read_fds, NULL, NULL, NULL)) {
      if (FD_ISSET(server_socket, &read_fds)) {
        if (!read(server_socket, buffer, sizeof(buffer))) {
          printf("\r[Server disconnect, exiting...]");
          kill(getpid(), SIGINT);
        }
        if (buffer[0] == '[') printf("\r%s\n>> ", buffer);
        else printf("\rReceived from %s\n>> ", buffer);
        fflush(stdout);
      }
      if (FD_ISSET(STDIN_FILENO, &read_fds)) {
        fgets(buffer, sizeof(buffer), stdin);
        *strchr(buffer, '\n') = 0;
        if (strcmp(buffer, "/exit") == 0) kill(getpid(), SIGINT);
        char to_send[BUFFER_SIZE];
        sprintf(to_send, "%s: %s", name, buffer);
        write(server_socket, to_send, sizeof(to_send));
      }
    }
  }
}

void client_sighandle(int sig) {
  if (sig == SIGINT) {
    if (write(server_socket, "/exit", 6) < 1)
      perror("[WRITING TO SOCKET]");
    close(server_socket);
    printf("\n");
    exit(1);
    }
}
