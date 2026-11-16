#include "codegen/calculator.h"
#include "codegen/stream.h"
#include "templates/network_common.h"

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>

int multiply(int a, int b) { return a * b; }

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

    struct mul_request_t *r = NULL;

    ser_buff_t *buff = NULL;
    init_serialized_buffer(&buff);

    // recv into full ser buffer capacity (default 100)
    int num_recv_bytes = recv_data(sock, buff->b, buff->size);

    // cursor must start at 0 so deserialize reads from beginning
    buff->next = 0;

    // deserializing header
    struct rpc_header_t *req_hdr = deserialize_rpc_header_t(buff);
    if (!req_hdr) {
      printf("Bad magic, dropping connection\n");

      free_serialize_buffer(buff);
      close(sock);
      continue;
    }
    printf("rpc_id %u, payload %u bytes\n", req_hdr->rpc_id,
           req_hdr->payload_len);

    r = deserialize_mul_request_t(buff);

    printf("server received data\n");
    printf("\t%d %d (should be 20 and 30)\n", r->a, r->b);

    int result = multiply(r->a, r->b);
    printf("\tresult is %d\n", result);

    struct mul_reply_t *res_obj = calloc(1, sizeof(struct mul_reply_t));
    res_obj->result = result;

    buff->next = 0;

    struct rpc_header_t res_hdr = *req_hdr; // same magic, version, rpc_id
    res_hdr.status = RPC_STATUS_SUCCESS;
    res_hdr.payload_len = serialized_size_mul_reply_t();

    serialize_rpc_header_t(&res_hdr, buff);
    serialize_mul_reply_t(res_obj, buff);

    send_data(sock, buff->b, buff->next);

    // free mul_request_t and mul_reply_t
    free(r);
    free(res_obj);
    free(req_hdr);
    free_serialize_buffer(buff);

    close(sock);
  }

  close(listen_sock);

  return EXIT_SUCCESS;
}
