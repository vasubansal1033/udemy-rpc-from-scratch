#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stream.h>

struct mul_request_t {
    int32_t a;
    int32_t b;
};

void serialize_mul_request_t(struct mul_request_t *obj, ser_buff_t *buff);
struct mul_request_t *deserialize_mul_request_t(ser_buff_t *buff);

struct mul_reply_t {
    int32_t result;
};

void serialize_mul_reply_t(struct mul_reply_t *obj, ser_buff_t *buff);
struct mul_reply_t *deserialize_mul_reply_t(ser_buff_t *buff);

struct person_t {
    int32_t id;
    char name[30];
};

void serialize_person_t(struct person_t *obj, ser_buff_t *buff);
struct person_t *deserialize_person_t(ser_buff_t *buff);

#endif // CALCULATOR_H
