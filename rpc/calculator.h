#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stream.h>

struct mul_request_t {
    int32_t a;
    int32_t b;
};

void serialize_mul_request_t(
  struct mul_request_t *obj,
  ser_buff_t *buff
) {
  
  // sentinel detection code
  if (!obj) {
    unsigned int sentienel = 0xFFFFFFFF;
    serialize_data(buff, (char *)&sentinel, sizeof(unsigned int));

    return;
  }

  serialize_data(buff, (char *)&obj->a, sizeof(int32_t))
  serialize_data(buff, (char *)&obj->b, sizeof(int32_t))
 
};

struct mul_request *deserialize_mul_request(
  ser_buff_t *buff
) {
    // sentinel detection code
    unsigned int sentinel = 0;
    deserialize_data((char *) &sentinel, buff, sizeof(unsigned int));
    
    if (sentinel == 0xFFFFFFFF) {
        return NULL;
    }

    serialize_buffer_skip_bytes(buff, -1 * sizeof(unsigned int));

    struct mul_request *obj = calloc(1, sizeof(struct mul_request));

    deserialize_data((char *)&obj->a, buff, sizeof(int32_t))
    deserialize_data((char *)&obj->b, buff, sizeof(int32_t))

    return obj;
  } 

struct mul_reply_t {
    int32_t result;
};

void serialize_mul_reply_t(
  struct mul_reply_t *obj,
  ser_buff_t *buff
) {
  
  // sentinel detection code
  if (!obj) {
    unsigned int sentienel = 0xFFFFFFFF;
    serialize_data(buff, (char *)&sentinel, sizeof(unsigned int));

    return;
  }

  serialize_data(buff, (char *)&obj->result, sizeof(int32_t))
 
};

struct mul_reply *deserialize_mul_reply(
  ser_buff_t *buff
) {
    // sentinel detection code
    unsigned int sentinel = 0;
    deserialize_data((char *) &sentinel, buff, sizeof(unsigned int));
    
    if (sentinel == 0xFFFFFFFF) {
        return NULL;
    }

    serialize_buffer_skip_bytes(buff, -1 * sizeof(unsigned int));

    struct mul_reply *obj = calloc(1, sizeof(struct mul_reply));

    deserialize_data((char *)&obj->result, buff, sizeof(int32_t))

    return obj;
  } 

struct person_t {
    int32_t id;
    char name[30];
};

void serialize_person_t(
  struct person_t *obj,
  ser_buff_t *buff
) {
  
  // sentinel detection code
  if (!obj) {
    unsigned int sentienel = 0xFFFFFFFF;
    serialize_data(buff, (char *)&sentinel, sizeof(unsigned int));

    return;
  }

  serialize_data(buff, (char *)&obj->id, sizeof(int32_t))
  serialize_data(buff, (char *)obj->name, sizeof(char) * 30)
 
};

struct person *deserialize_person(
  ser_buff_t *buff
) {
    // sentinel detection code
    unsigned int sentinel = 0;
    deserialize_data((char *) &sentinel, buff, sizeof(unsigned int));
    
    if (sentinel == 0xFFFFFFFF) {
        return NULL;
    }

    serialize_buffer_skip_bytes(buff, -1 * sizeof(unsigned int));

    struct person *obj = calloc(1, sizeof(struct person));

    deserialize_data((char *)&obj->id, buff, sizeof(int32_t))
    deserialize_data((char *)obj->name, buff, sizeof(char) * 30)

    return obj;
  } 

#endif // CALCULATOR_H
