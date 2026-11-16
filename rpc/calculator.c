#include "calculator.h"

#include <stdlib.h>

void serialize_mul_request_t(struct mul_request_t *obj, ser_buff_t *buff) {
  // sentinel detection code
  if (!obj) {
    unsigned int sentinel = 0xFFFFFFFF;
    serialize_data(buff, (char *)&sentinel, sizeof(unsigned int));

    return;
  }

  serialize_data(buff, (char *)&obj->a, sizeof(obj->a));
  serialize_data(buff, (char *)&obj->b, sizeof(obj->b));
}

struct mul_request_t *deserialize_mul_request_t(ser_buff_t *buff) {
  // sentinel detection code
  unsigned int sentinel = 0;
  deserialize_data((char *)&sentinel, buff, sizeof(unsigned int));

  if (sentinel == 0xFFFFFFFF) {
    return NULL;
  }

  serialize_buffer_skip_bytes(buff, -(int)sizeof(unsigned int));

  struct mul_request_t *obj = calloc(1, sizeof(struct mul_request_t));

  deserialize_data((char *)&obj->a, buff, sizeof(obj->a));
  deserialize_data((char *)&obj->b, buff, sizeof(obj->b));

  return obj;
}

void serialize_mul_reply_t(struct mul_reply_t *obj, ser_buff_t *buff) {
  // sentinel detection code
  if (!obj) {
    unsigned int sentinel = 0xFFFFFFFF;
    serialize_data(buff, (char *)&sentinel, sizeof(unsigned int));

    return;
  }

  serialize_data(buff, (char *)&obj->result, sizeof(obj->result));
}

struct mul_reply_t *deserialize_mul_reply_t(ser_buff_t *buff) {
  // sentinel detection code
  unsigned int sentinel = 0;
  deserialize_data((char *)&sentinel, buff, sizeof(unsigned int));

  if (sentinel == 0xFFFFFFFF) {
    return NULL;
  }

  serialize_buffer_skip_bytes(buff, -(int)sizeof(unsigned int));

  struct mul_reply_t *obj = calloc(1, sizeof(struct mul_reply_t));

  deserialize_data((char *)&obj->result, buff, sizeof(obj->result));

  return obj;
}

void serialize_person_t(struct person_t *obj, ser_buff_t *buff) {
  // sentinel detection code
  if (!obj) {
    unsigned int sentinel = 0xFFFFFFFF;
    serialize_data(buff, (char *)&sentinel, sizeof(unsigned int));

    return;
  }

  serialize_data(buff, (char *)&obj->id, sizeof(obj->id));
  serialize_data(buff, (char *)&obj->name, sizeof(obj->name));
}

struct person_t *deserialize_person_t(ser_buff_t *buff) {
  // sentinel detection code
  unsigned int sentinel = 0;
  deserialize_data((char *)&sentinel, buff, sizeof(unsigned int));

  if (sentinel == 0xFFFFFFFF) {
    return NULL;
  }

  serialize_buffer_skip_bytes(buff, -(int)sizeof(unsigned int));

  struct person_t *obj = calloc(1, sizeof(struct person_t));

  deserialize_data((char *)&obj->id, buff, sizeof(obj->id));
  deserialize_data((char *)&obj->name, buff, sizeof(obj->name));

  return obj;
}

