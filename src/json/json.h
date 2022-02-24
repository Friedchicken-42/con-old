#ifndef JSON_H
#define JSON_H

#include "../on.h"

Object *json_load(const char* filename);
Object *json_loads(const char* data);

void json_dump(Object *o, const char* filename);
char *json_dumps(Object *o);

#endif
