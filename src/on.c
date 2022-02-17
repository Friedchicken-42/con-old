#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "on.h"

Object *on_create() {
    Object *o = (Object*)malloc(sizeof(Object));
    if(o == NULL) exit(-1);
    o->key = NULL;
    o->value = NULL;
    o->next = NULL;
    o->prev = NULL;
    o->type = CON_EMPTY;
    return o;
}

void on_add_key(Object *o, const char *key) {
    int length = 1;
    while(key[length] != '\0') length++;
    o->key = malloc(length + 1);
    if(o->key == NULL) exit(-1);
    strcpy(o->key, key);
}

void on_add_string(Object *o, char *data) {
    int length = 1;
    while(data[length] != '\0') {
        length++;
    }

    o->value = (char*)malloc(length + 1);
    if(o->value == NULL) exit(-1);
    strcpy(o->value, data);
    ((char*)o->value)[length] = '\0';
}

#define _on_malloc(T) do{\
    o->value = (T*)malloc(sizeof(T));\
    if(o->value == NULL) exit(-1);\
    if(data) *(T*)o->value = *(T*)data; \
}while(0);

void on_add_value(Object *o, void *data) {
    switch(o->type) {
        case CON_INTEGER:
            _on_malloc(int);
            break;
        case CON_FLOAT:
            _on_malloc(float);
            break;
        case CON_STRING:
            on_add_string(o, data);
            break;
        case CON_TRUE:
        case CON_FALSE:
        case CON_NULL:
        case CON_EMPTY:
            break;
        case CON_ARRAY:
        case CON_OBJECT:
            o->value = on_create();
            break;
    }
}

Object *on_get_on(Object *o, const char *key) {
    Object *curr = o;
    while(curr->prev) curr = curr->prev;
    while(curr) {
        if(strcmp(curr->key, key) == 0) return curr;
        curr = curr->next;
    }
    return NULL;
}

void *on_get(Object *o, const char *key) {
    Object *x = on_get_on(o, key);
    if(x) return x->value;
    return NULL;
}

void *on_get_array(Object *o, int index) {
    Object *curr = o;
    for(int i = 0; i < index; i++) {
        if(curr == NULL) return NULL;
        curr = curr->next;
    }
    return curr->value;
}

void on_free(Object *o) {
    if(o->key) free(o->key);

    if(o->type == CON_OBJECT || o->type == CON_ARRAY) on_clean(o->value);
    else free(o->value);

    free(o);
}

void on_remove(Object **o, Object *del) {
    if(*o == NULL || del == NULL) return;

    if(*o == del) *o = del->next;
    if(del->next != NULL) del->next->prev = del->prev;
    if(del->prev != NULL) del->prev->next = del->next;

    on_free(del);
}

int on_add(Object *o, const char *key, void *data, enum ValueType type) {
    if(o == NULL) return -1;

    Object *curr = o;

    if(o->type != CON_EMPTY) {
        if(o->key) {
            Object *temp = on_get_on(o, key);
            if(temp) on_remove(&o, temp);
        }
        while(curr->next) curr = curr->next;
        curr->next = on_create();
        curr->next->prev = curr;
        curr = curr->next;
    }

    if(key) on_add_key(curr, key);
    curr->type = type;

    on_add_value(curr, data);

    return 0;
}

void on_clean(Object *o) {
    while(o) on_remove(&o, o);
}
