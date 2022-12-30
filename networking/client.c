#include "network_common.h"

#include <unistd.h>

int main(int argc, char *argv[]) {

  char *IP_ADDR = "127.0.0.1";
  int PORT = 8080;

  int sock = create_send_socket(IP_ADDR, PORT);

  char *buff = "Hello world!";
  int buff_size = 30;
  send_data(sock, buff, buff_size);

  close(sock);

  return EXIT_SUCCESS;
}
