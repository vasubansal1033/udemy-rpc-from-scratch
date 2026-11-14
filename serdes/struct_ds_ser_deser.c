#include "stream.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ser_buff_t *buff;

struct person_t {
  char name[30];
  int age;
  int weight;
};

void serialize_person_t(struct person_t *obj, ser_buff_t *b) {
  // sentinel detection code
  if (!obj) {
    unsigned int sentinel = 0xFFFFFFFF;
    serialize_data(b, (char *)&sentinel, sizeof(unsigned int));

    return;
  }

  serialize_data(b, (char *)obj->name, sizeof(char) * 30);
  serialize_data(b, (char *)&obj->age, sizeof(int));
  serialize_data(b, (char *)&obj->weight, sizeof(int));
}

struct person_t *deserialize_person_t(ser_buff_t *buff) {
  // sentinel detection code
  unsigned int sentinel = 0;
  deserialize_data((char *)&sentinel, buff, sizeof(unsigned int));
  if (sentinel == 0xFFFFFFFF) {
    return NULL;
  }

  serialize_buffer_skip_bytes(buff, -1 * (int)sizeof(unsigned int));

  struct person_t *p = calloc(1, sizeof(struct person_t));
  deserialize_data((char *)p->name, buff, sizeof(char) * 30);
  deserialize_data((char *)&p->age, buff, sizeof(int));
  deserialize_data((char *)&p->weight, buff, sizeof(int));

  return p;
}

int main(void) {
  buff = NULL;
  init_serialized_buffer(&buff);

  struct person_t *p1 = (struct person_t *)calloc(1, sizeof(struct person_t));
  // p1->name = "Vasu";
  strcpy(p1->name, "Vasu");
  p1->age = 28;
  p1->weight = 85;

  serialize_person_t(p1, buff);

  /* reset cursor to read from start */
  buff->next = 0;

  struct person_t *p_deser = deserialize_person_t(buff);

  printf("Printing deserialized person_t\n");
  printf("\t%s (should be Vasu)\n", p_deser->name);
  printf("\t%d (should be 28)\n", p_deser->age);
  printf("\t%d (should be 85)\n", p_deser->weight);

  return EXIT_SUCCESS;
}
