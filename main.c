#include "stream.h"

#include <stdio.h>
#include <string.h>

int main(void) {
  ser_buff_t *buff = NULL;
  init_serialized_buffer(&buff);

  int n = 42;
  char msg[] = "hello";
  float f = 3.14f;

  serialize_data(buff, (char *)&n, sizeof(n));
  serialize_data(buff, msg, sizeof(msg));
  serialize_data(buff, (char *)&f, sizeof(f));

  printf("wrote %d bytes (capacity %d)\n", buff->next, buff->size);

  /* reset cursor to read from start */
  buff->next = 0;

  int n_out = 0;
  char msg_out[sizeof(msg)] = {0};
  float f_out = 0.0f;

  deserialize_data((char *)&n_out, buff, sizeof(n_out));
  deserialize_data(msg_out, buff, sizeof(msg_out));
  deserialize_data((char *)&f_out, buff, sizeof(f_out));

  printf("int:   %d (want 42)\n", n_out);
  printf("str:   %s (want hello)\n", msg_out);
  printf("float: %f (want 3.14)\n", f_out);

  if (n_out == 42 && strcmp(msg_out, "hello") == 0 && f_out > 3.13f &&
      f_out < 3.15f) {
    printf("PASS\n");
  } else {
    printf("FAIL\n");
  }

  free_serialize_buffer(buff);
  return 0;
}
