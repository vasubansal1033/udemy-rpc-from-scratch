#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stdint.h>
#include <stdbool.h>
#include "stream.h"

// ---------- rpc header ----------
#define RPC_MAGIC          "V4$U8@N5@!"
#define RPC_MAGIC_LEN      10
#define RPC_VERSION        1
#define RPC_STATUS_FAILURE 0
#define RPC_STATUS_SUCCESS 1
#define RPC_HEADER_SIZE    22

struct rpc_header_t {
    char magic[RPC_MAGIC_LEN];
    uint16_t version;
    uint32_t rpc_id;      // server dispatches on this
    uint16_t status;      // RPC_STATUS_FAILURE / RPC_STATUS_SUCCESS
    uint32_t payload_len; // bytes following the header
};

void serialize_rpc_header_t(struct rpc_header_t *obj, ser_buff_t *buff);
// returns NULL if magic does not match
struct rpc_header_t *deserialize_rpc_header_t(ser_buff_t *buff);

// ---------- sentinel ----------
// Every object is prefixed with a marker so field data can never be
// mistaken for NULL: SENTINEL_PRESENT = object follows, SENTINEL_NULL = NULL.
#define SENTINEL_PRESENT 0x00000000u
#define SENTINEL_NULL    0xFFFFFFFFu
#define SENTINEL_SIZE    4

void serialize_sentinel(ser_buff_t *buff, bool is_null);
// returns true if the next object is NULL
bool deserialize_sentinel(ser_buff_t *buff);

// ---------- types ----------
struct mul_request_t {
    int32_t a;
    int32_t b;
};

void serialize_mul_request_t(struct mul_request_t *obj, ser_buff_t *buff);
struct mul_request_t *deserialize_mul_request_t(ser_buff_t *buff);
// bytes written by serialize_mul_request_t for a non-NULL object (sentinel + fields)
uint32_t serialized_size_mul_request_t(void);

struct mul_reply_t {
    int32_t result;
};

void serialize_mul_reply_t(struct mul_reply_t *obj, ser_buff_t *buff);
struct mul_reply_t *deserialize_mul_reply_t(ser_buff_t *buff);
// bytes written by serialize_mul_reply_t for a non-NULL object (sentinel + fields)
uint32_t serialized_size_mul_reply_t(void);

struct person_t {
    int32_t id;
    char name[30];
};

void serialize_person_t(struct person_t *obj, ser_buff_t *buff);
struct person_t *deserialize_person_t(ser_buff_t *buff);
// bytes written by serialize_person_t for a non-NULL object (sentinel + fields)
uint32_t serialized_size_person_t(void);

#endif // CALCULATOR_H
