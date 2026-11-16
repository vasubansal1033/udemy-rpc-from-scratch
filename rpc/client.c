#include "codegen/calculator.h"
#include "codegen/stream.h"
#include "templates/network_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

  char *IP_ADDR = "127.0.0.1";
  int PORT = 8080;

  int sock = create_send_socket(IP_ADDR, PORT);

  struct mul_request_t *r = calloc(1, sizeof(struct mul_request_t));
  r->a = 20;
  r->b = 30;

  // Send mul_request_t struct
  ser_buff_t *buff = NULL;
  init_serialized_buffer(&buff);

  // header first: payload size is known up front from the generated size fn
  struct rpc_header_t req_hdr = {0};
  memcpy(req_hdr.magic, RPC_MAGIC, RPC_MAGIC_LEN);
  req_hdr.version = RPC_VERSION;
  req_hdr.rpc_id = 1; // Multiply (id from mult.yaml)
  req_hdr.status = RPC_STATUS_SUCCESS;
  req_hdr.payload_len = serialized_size_mul_request_t();

  serialize_rpc_header_t(&req_hdr, buff);
  serialize_mul_request_t(r, buff);

  // buff->next = bytes written by serialize (name+age+occ+weight ≈ 72)
  int buff_size = buff->next;

  printf("Sending mul_request_t datastructure (%d bytes)\n", buff_size);
  send_data(sock, buff->b, buff_size);

  // reset buffer
  serialize_buffer_skip_bytes(buff, -1 * buff_size);

  // get response header
  recv_data(sock, buff->b, RPC_HEADER_SIZE);
  struct rpc_header_t *res_hdr = deserialize_rpc_header_t(buff);
  if (!res_hdr || res_hdr->status != RPC_STATUS_SUCCESS) {
    printf("RPC failed (bad magic or failure status)\n");

    free(res_hdr);
    free(r);
    free_serialize_buffer(buff);
    close(sock);

    return EXIT_FAILURE;
  }

  // get response payload right
  int response_size = res_hdr->payload_len;
  int recv_data_size = recv_data(sock, buff->b + buff->next, response_size);

  printf("Received mul_reply_t datastructure (%d bytes)\n", response_size);

  struct mul_reply_t *res = deserialize_mul_reply_t(buff);

  printf("Result is %d\n", res->result);

  free(res);
  free(res_hdr);
  free(r);
  free_serialize_buffer(buff);

  close(sock);

  return EXIT_SUCCESS;
}
