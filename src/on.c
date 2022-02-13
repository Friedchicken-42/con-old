#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "on.h"

Object *on_create() {
    Object *o = (Object*)malloc(sizeof(Object));
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
    strcpy(o->key, key);
}

#define _on_malloc(T) do{\
    o->value = (T*)malloc(sizeof(T));\
    if(data) *(T*)o->value = *(T*)data; \
}while(0);

void on_add_string(Object *o, char *data) {
    int length = 1;
    while(data[length] != '\0') {
        length++;
    }

    o->value = (char*)malloc(length);
    strcpy(o->value, data);
}

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

void on_free(Object *o) {
    free(o->value);
    free(o);
}

void on_remove(Object *o) {
    if(o->prev != NULL) o->prev->next = o->next;
    if(o->next != NULL) o->next->prev = o->prev;

    if(o->type == CON_ARRAY || o->type == CON_OBJECT) {
        Object *v = o->value;
        while(v) {
            on_remove(v);
            v = v->next;
        }
    }

    on_free(o);
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

void *on_get_array(Object *o, int index) {
    Object *curr = o;
    for(int i = 0; i < index; i++) {
        if(curr == NULL) return NULL;
        curr = curr->next;
    }
    return curr->value;
}

int on_add(Object *o, const char *key, void *data, enum ValueType type) {
    if(o == NULL) return -1;

    Object *curr = o;

    if(o->type != CON_EMPTY) {
        if(o->key) {
            Object *temp = on_get_on(o, key);
            if(temp) on_remove(temp);
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

void *on_get(Object *o, const char *key) {
    Object *x = on_get_on(o, key);
    if(x) return x->value;
    return NULL;
}

void on_del(Object *o, const char *key) {
    Object *x = on_get_on(o, key);
    if(x) on_remove(x);
}
