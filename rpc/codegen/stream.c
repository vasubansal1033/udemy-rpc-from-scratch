#include "stream.h"

#include <stdlib.h>
#include <string.h>

void init_serialized_buffer(ser_buff_t **b) {
  *b = (ser_buff_t *)calloc(1, sizeof(ser_buff_t));

  (*b)->b = calloc(1, SERIALIZE_BUFFER_DEFAULT_SIZE);
  (*b)->size = SERIALIZE_BUFFER_DEFAULT_SIZE;
  (*b)->next = 0;
}

void serialize_data(ser_buff_t *buff, char *data, int nbytes) {
  int available_bytes =
      buff->size - buff->next; // free space in serialized_buffer
  char is_resize = 0;

  while (available_bytes < nbytes) {
    buff->size = 2 * buff->size;
    available_bytes = buff->size - buff->next;
    is_resize = 1;
  }

  // if it already fits: no realloc needed
  if (is_resize == 0) {
    memcpy((char *)buff->b + buff->next, data, nbytes);
    buff->next += nbytes;

    return;
  }

  // resize is needed: reallocation necessary with new updated size
  buff->b = realloc(buff->b, buff->size);
  memcpy((char *)buff->b + buff->next, data, nbytes);
  buff->next += nbytes;
}

void deserialize_data(char *dst, ser_buff_t *buff, int size) {
  memcpy(dst, buff->b + buff->next, size);
  buff->next += size;
}

void serialize_buffer_skip_bytes(ser_buff_t *buff, int skip_size) {
  if (buff->next + skip_size < buff->size && buff->next + skip_size >= 0) {
    buff->next += skip_size;
  }
}

void free_serialize_buffer(ser_buff_t *buff) {
  free(buff->b);
  free(buff);
}
