#include "calculator.h"

#include <stdlib.h>

// Every object is prefixed with a marker so field data can never be
// mistaken for NULL: SENTINEL_PRESENT = object follows, SENTINEL_NULL = NULL.
#define SENTINEL_PRESENT 0x00000000u
#define SENTINEL_NULL    0xFFFFFFFFu

void serialize_mul_request_t(struct mul_request_t *obj, ser_buff_t *buff) {
  unsigned int sentinel = obj ? SENTINEL_PRESENT : SENTINEL_NULL;
  serialize_data(buff, (char *)&sentinel, sizeof(unsigned int));

  if (!obj) {
    return;
  }

  serialize_data(buff, (char *)&obj->a, sizeof(obj->a));
  serialize_data(buff, (char *)&obj->b, sizeof(obj->b));
}

struct mul_request_t *deserialize_mul_request_t(ser_buff_t *buff) {
  unsigned int sentinel = 0;
  deserialize_data((char *)&sentinel, buff, sizeof(unsigned int));

  if (sentinel == SENTINEL_NULL) {
    return NULL;
  }

  struct mul_request_t *obj = calloc(1, sizeof(struct mul_request_t));

  deserialize_data((char *)&obj->a, buff, sizeof(obj->a));
  deserialize_data((char *)&obj->b, buff, sizeof(obj->b));

  return obj;
}

void serialize_mul_reply_t(struct mul_reply_t *obj, ser_buff_t *buff) {
  unsigned int sentinel = obj ? SENTINEL_PRESENT : SENTINEL_NULL;
  serialize_data(buff, (char *)&sentinel, sizeof(unsigned int));

  if (!obj) {
    return;
  }

  serialize_data(buff, (char *)&obj->result, sizeof(obj->result));
}

struct mul_reply_t *deserialize_mul_reply_t(ser_buff_t *buff) {
  unsigned int sentinel = 0;
  deserialize_data((char *)&sentinel, buff, sizeof(unsigned int));

  if (sentinel == SENTINEL_NULL) {
    return NULL;
  }

  struct mul_reply_t *obj = calloc(1, sizeof(struct mul_reply_t));

  deserialize_data((char *)&obj->result, buff, sizeof(obj->result));

  return obj;
}

void serialize_person_t(struct person_t *obj, ser_buff_t *buff) {
  unsigned int sentinel = obj ? SENTINEL_PRESENT : SENTINEL_NULL;
  serialize_data(buff, (char *)&sentinel, sizeof(unsigned int));

  if (!obj) {
    return;
  }

  serialize_data(buff, (char *)&obj->id, sizeof(obj->id));
  serialize_data(buff, (char *)&obj->name, sizeof(obj->name));
}

struct person_t *deserialize_person_t(ser_buff_t *buff) {
  unsigned int sentinel = 0;
  deserialize_data((char *)&sentinel, buff, sizeof(unsigned int));

  if (sentinel == SENTINEL_NULL) {
    return NULL;
  }

  struct person_t *obj = calloc(1, sizeof(struct person_t));

  deserialize_data((char *)&obj->id, buff, sizeof(obj->id));
  deserialize_data((char *)&obj->name, buff, sizeof(obj->name));

  return obj;
}

