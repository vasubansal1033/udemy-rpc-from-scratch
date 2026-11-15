#include "nested_pointer_struct_serdes.h"
#include "network_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

  char *IP_ADDR = "127.0.0.1";
  int PORT = 8080;

  int sock = create_send_socket(IP_ADDR, PORT);

  // char *buff = "Hello world!";

  struct person_t *p = calloc(1, sizeof(struct person_t));
  struct occupation_t *o = calloc(1, sizeof(struct occupation_t));

  strcpy(p->name, "Vasu");
  p->age = 28;
  p->weight = 85;

  strcpy(o->dept_name, "AI Platform");
  o->employee_code = 99;
  p->occ = o;

  // Send person_t struct from nested_pointer_struct_serdes
  ser_buff_t *buff = NULL;
  init_serialized_buffer(&buff);

  serialize_person_t(p, buff);

  // buff->next = bytes written by serialize (name+age+occ+weight ≈ 72)
  int buff_size = buff->next;

  printf("Sending person_t datastructure (%d bytes)\n", buff_size);
  send_data(sock, buff->b, buff_size);

  free_serialize_buffer(buff);
  close(sock);

  return EXIT_SUCCESS;
}
