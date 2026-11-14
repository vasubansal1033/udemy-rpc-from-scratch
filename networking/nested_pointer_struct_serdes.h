#include "../serdes/stream.h"

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

void serialize_occupation_t(struct occupation_t *obj, ser_buff_t *buff);
struct occupation_t *deserialize_occupation_t(ser_buff_t *buff);
void serialize_person_t(struct person_t *obj, ser_buff_t *buff);
struct person_t *deserialize_person_t(ser_buff_t *buff);
