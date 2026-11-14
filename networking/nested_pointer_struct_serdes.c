#include "nested_pointer_struct_serdes.h"
#include "../serdes/stream.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void serialize_occupation_t(struct occupation_t *obj, ser_buff_t *buff) {
  // sentinel detection code
  if (!obj) {
    unsigned int sentinel = 0xFFFFFFFF;
    serialize_data(buff, (char *)&sentinel, sizeof(unsigned int));

    return;
  }

  serialize_data(buff, (char *)obj->dept_name, sizeof(char) * 30);
  serialize_data(buff, (char *)&obj->employee_code, sizeof(int));
}

struct occupation_t *deserialize_occupation_t(ser_buff_t *buff) {
  // sentinel detection code
  unsigned int sentinel = 0;
  deserialize_data((char *)&sentinel, buff, sizeof(unsigned int));

  if (sentinel == 0xFFFFFFFF) {
    return NULL;
  }

  serialize_buffer_skip_bytes(buff, -1 * sizeof(unsigned int));

  struct occupation_t *o = calloc(1, sizeof(occupation_t));
  deserialize_data((char *)o->dept_name, buff, sizeof(char) * 30);
  deserialize_data((char *)&o->employee_code, buff, sizeof(int));

  return o;
}

void serialize_person_t(struct person_t *obj, ser_buff_t *buff) {
  // sentinel detection code
  if (!obj) {
    unsigned int sentinel = 0xFFFFFFFF;
    serialize_data(buff, (char *)&sentinel, sizeof(unsigned int));

    return;
  }

  serialize_data(buff, (char *)obj->name, sizeof(char) * 30);
  serialize_data(buff, (char *)&obj->age, sizeof(int));
  serialize_occupation_t(obj->occ, buff);
  serialize_data(buff, (char *)&obj->weight, sizeof(int));
}

struct person_t *deserialize_person_t(ser_buff_t *buff) {
  // sentinel detection code
  unsigned int sentinel = 0;
  deserialize_data((char *)&sentinel, buff, sizeof(unsigned int));
  if (sentinel == 0xFFFFFFFF) {
    return NULL;
  }

  serialize_buffer_skip_bytes(buff, -1 * sizeof(unsigned int));

  struct person_t *p = calloc(1, sizeof(struct person_t));

  deserialize_data((char *)p->name, buff, sizeof(char) * 30);
  deserialize_data((char *)&p->age, buff, sizeof(int));
  p->occ = deserialize_occupation_t(buff);
  deserialize_data((char *)&p->weight, buff, sizeof(int));

  return p;
}
