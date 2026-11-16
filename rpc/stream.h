#ifndef STREAM_H
#define STREAM_H

#define SERIALIZE_BUFFER_DEFAULT_SIZE 1000

typedef struct serialized_buffer {
  char *b;           // points to the start of the serialized_buffer
  unsigned int size; // total capacity of the serialized_buffer
  unsigned int next; // cursor offset: byte offset where next read/write happens
} ser_buff_t;

void init_serialized_buffer(ser_buff_t **b);
void serialize_data(ser_buff_t *buff, char *data, int nbytes);
void deserialize_data(char *dst, ser_buff_t *buff, int size);
void serialize_buffer_skip_bytes(ser_buff_t *buff, int skip_size);
void free_serialize_buffer(ser_buff_t *buff);

#endif /* STREAM_H */
