#ifndef ON_H
#define ON_H

enum ValueType {
    CON_EMPTY,
    CON_STRING,
    CON_INTEGER,
    CON_FLOAT,
    CON_OBJECT,
    CON_ARRAY,
    CON_TRUE,
    CON_FALSE,
    CON_NULL,
};

typedef struct Object_t Object;

struct Object_t {
    char *key;
    void *value;
    enum ValueType type;
    enum ValueType parent;
    Object *next;
    Object *prev;
};

Object *on_create_on();
Object *on_create_array();
Object *on_get_on(Object *o, const char *key);
void *on_get(Object *o, const char *key);
void *on_get_array(Object *o, int index);
int on_add(Object *o, const char *key, void *data, enum ValueType type);
void on_clean(Object *o);

#endif
