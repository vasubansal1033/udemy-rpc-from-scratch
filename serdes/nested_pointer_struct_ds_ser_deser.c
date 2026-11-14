#include "stream.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXIT_SUCCESS 0

ser_buff_t *buff;

struct occupation_t {
  char dept_name[30];
  int employee_code;
} occupation_t;

struct person_t {
  char name[30];
  int age;
  struct occupation_t *occ;
  int weight;
};

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

int main(int argc, char *argv[]) {
  buff = NULL;
  init_serialized_buffer(&buff);

  struct person_t *p = (struct person_t *)calloc(1, sizeof(struct person_t));
  strcpy(p->name, "Vasu");
  p->weight = 85;
  p->age = 28;

  struct occupation_t *o =
      (struct occupation_t *)calloc(1, sizeof(struct occupation_t));
  strcpy(o->dept_name, "AI Platform");
  o->employee_code = 99;

  p->occ = o;

  // serialize person_t
  serialize_person_t(p, buff);

  /* reset cursor to intiali position for deserialization */
  buff->next = 0;

  struct person_t *p_deser = deserialize_person_t(buff);

  printf("Printing deserialized person_t\n");
  printf("\t%s (should be Vasu)\n", p_deser->name);
  printf("\t%d (should be 85)\n", p_deser->weight);
  printf("\t%d (should be 28)\n", p_deser->age);
  printf("\t%s (should be AI Platform)\n", p_deser->occ->dept_name);
  printf("\t%d (should be 99)\n", p_deser->occ->employee_code);

  return EXIT_SUCCESS;
}
