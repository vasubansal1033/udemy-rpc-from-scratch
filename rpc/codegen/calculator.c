#include "calculator.h"

#include <stdlib.h>
#include <string.h>

// ---------- rpc header ----------
void serialize_rpc_header_t(struct rpc_header_t *obj, ser_buff_t *buff) {
  serialize_data(buff, (char *)&obj->magic, sizeof(obj->magic));
  serialize_data(buff, (char *)&obj->version, sizeof(obj->version));
  serialize_data(buff, (char *)&obj->rpc_id, sizeof(obj->rpc_id));
  serialize_data(buff, (char *)&obj->status, sizeof(obj->status));
  serialize_data(buff, (char *)&obj->payload_len, sizeof(obj->payload_len));
}

struct rpc_header_t *deserialize_rpc_header_t(ser_buff_t *buff) {
  struct rpc_header_t *obj = calloc(1, sizeof(struct rpc_header_t));

  deserialize_data((char *)&obj->magic, buff, sizeof(obj->magic));
  if (memcmp(obj->magic, RPC_MAGIC, RPC_MAGIC_LEN) != 0) {
    free(obj);
    return NULL;
  }

  deserialize_data((char *)&obj->version, buff, sizeof(obj->version));
  deserialize_data((char *)&obj->rpc_id, buff, sizeof(obj->rpc_id));
  deserialize_data((char *)&obj->status, buff, sizeof(obj->status));
  deserialize_data((char *)&obj->payload_len, buff, sizeof(obj->payload_len));

  return obj;
}

// ---------- sentinel ----------
void serialize_sentinel(ser_buff_t *buff, bool is_null) {
  unsigned int sentinel = is_null ? SENTINEL_NULL : SENTINEL_PRESENT;
  serialize_data(buff, (char *)&sentinel, sizeof(unsigned int));
}

bool deserialize_sentinel(ser_buff_t *buff) {
  unsigned int sentinel = 0;
  deserialize_data((char *)&sentinel, buff, sizeof(unsigned int));

  return sentinel == SENTINEL_NULL;
}

// ---------- types ----------
void serialize_mul_request_t(struct mul_request_t *obj, ser_buff_t *buff) {
  serialize_sentinel(buff, obj == NULL);
  if (!obj) {
    return;
  }

  serialize_data(buff, (char *)&obj->a, sizeof(obj->a));
  serialize_data(buff, (char *)&obj->b, sizeof(obj->b));
}

struct mul_request_t *deserialize_mul_request_t(ser_buff_t *buff) {
  if (deserialize_sentinel(buff)) {
    return NULL;
  }

  struct mul_request_t *obj = calloc(1, sizeof(struct mul_request_t));

  deserialize_data((char *)&obj->a, buff, sizeof(obj->a));
  deserialize_data((char *)&obj->b, buff, sizeof(obj->b));

  return obj;
}

void serialize_mul_reply_t(struct mul_reply_t *obj, ser_buff_t *buff) {
  serialize_sentinel(buff, obj == NULL);
  if (!obj) {
    return;
  }

  serialize_data(buff, (char *)&obj->result, sizeof(obj->result));
}

struct mul_reply_t *deserialize_mul_reply_t(ser_buff_t *buff) {
  if (deserialize_sentinel(buff)) {
    return NULL;
  }

  struct mul_reply_t *obj = calloc(1, sizeof(struct mul_reply_t));

  deserialize_data((char *)&obj->result, buff, sizeof(obj->result));

  return obj;
}

void serialize_person_t(struct person_t *obj, ser_buff_t *buff) {
  serialize_sentinel(buff, obj == NULL);
  if (!obj) {
    return;
  }

  serialize_data(buff, (char *)&obj->id, sizeof(obj->id));
  serialize_data(buff, (char *)&obj->name, sizeof(obj->name));
}

struct person_t *deserialize_person_t(ser_buff_t *buff) {
  if (deserialize_sentinel(buff)) {
    return NULL;
  }

  struct person_t *obj = calloc(1, sizeof(struct person_t));

  deserialize_data((char *)&obj->id, buff, sizeof(obj->id));
  deserialize_data((char *)&obj->name, buff, sizeof(obj->name));

  return obj;
}

