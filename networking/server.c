#include "network_common.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

  char *IP_ADDR = "127.0.0.1";
  int PORT = 8080;

  int sock = create_listening_socket(IP_ADDR, PORT);

  char *buff = calloc(1, sizeof(char) * 30);
  int recv_size = 30;

  int num_recv_bytes = recv_data(sock, buff, recv_size);

  printf("server received: %s\n", buff);
  return EXIT_SUCCESS;
}
