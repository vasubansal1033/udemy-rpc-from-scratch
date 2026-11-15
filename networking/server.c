#include "../serdes/stream.h"
#include "nested_pointer_struct_serdes.h"
#include "network_common.h"

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

  char *IP_ADDR = "127.0.0.1";
  int PORT = 8080;

  int listen_sock = create_listening_socket(IP_ADDR, PORT);
  if (listen_sock < 0) {
    return EXIT_FAILURE;
  }

  for (;;) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int sock =
        accept(listen_sock, (struct sockaddr *)&client_addr, &client_addr_len);
    if (sock < 0) {
      perror("Failed to accept connection");
      continue;
    }

    printf("Client connected succesfully\n");

    struct person_t *p = NULL;

    ser_buff_t *buff = NULL;
    init_serialized_buffer(&buff);

    // recv into full ser buffer capacity (default 100), not just name's 30
    // bytes
    int num_recv_bytes = recv_data(sock, buff->b, buff->size);

    // cursor must start at 0 so deserialize reads from beginning
    buff->next = 0;
    p = deserialize_person_t(buff);

    printf("server received data\n");
    printf("\t%s (should be Vasu)\n", p->name);
    printf("\t%d (should be 28)\n", p->age);
    printf("\t%d (should be 85)\n", p->weight);
    printf("\t%s (should be AI platform)\n", p->occ->dept_name);
    printf("\t%d (should be 99)\n", p->occ->employee_code);

    free_person_t(p);
    free_serialize_buffer(buff);
    close(sock);
  }

  close(listen_sock);

  return EXIT_SUCCESS;
}
