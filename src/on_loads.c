#include "on.h"
#include <stdio.h>
#include <stdlib.h>

int on_whitespace(const char *data) {
    int length = 0;
    while(1) {
        switch(data[length]) {
            case ' ':
            case '\t':
            case '\n':
                length++;
                break;
            default:
                return length;
        }
    }
}

char *on_read_string(const char *data, int *length) {
    *length = 1;

    while(data[*length] != '"') {
        if(data[*length] == '\\') *length += 1;
        *length += 1;
    }

    char *str = (char*)malloc(*length);
    for(int i = 0; i < *length - 1; i++) {
        str[i] = data[i + 1];
    }
    str[*length - 1] = '\0';
    *length += 1;

    return str;
}

int on_read_single(const char *data, const char *string) {
    int i = 0;

    while(string[i] != '\0') {
        if (data[i] != string[i]) return -1;
        i++;
    }

    return i;
}

float *on_read_float(const char *data, int length) {
    float *number = (float*)malloc(sizeof(float));
    *number = 0;
    int radix = 0;

    for(int i = 0; i < length; i++) {
        if(radix > 0) radix *= 10;
        if(data[i] == '.') radix = 1;
        else *number = *number * 10 + (int)(data[i] - '0');
    }
    *number /= radix;

    return number;
}

int *on_read_int(const char *data, int length) {
    int *number = (int*)malloc(sizeof(int));
    *number = 0;

    for(int i = 0; i < length; i++) {
        *number = *number * 10 + (int)(data[i] - '0');
    }

    return number;
}

void *on_read_number(const char *data, int *length, enum ValueType *type) {
    int size = 1;
    char isFloat = 0;

    while(1) {
        if(data[size] == '.') {
            if (isFloat) {
                *length = -1;
                return NULL;
            }
            isFloat = 1;
        } else if (data[size] < '0' || data[size] > '9') {
            break;
        }
        size++;
    }

    *length = size;

    if (isFloat) {
        *type = CON_FLOAT;
        return on_read_float(data, size);
    } else {
        *type = CON_INTEGER;
        return on_read_int(data, size);
    }
}

void *on_read_value(const char *data, int *length, enum ValueType *type) {
    *length = 0;

    switch(data[0]) {
        case '"':
            *type = CON_STRING; 
            return on_read_string(data, length);
        case 't':
            *type = CON_TRUE;
            *length = on_read_single(data, "true");
            break;
        case 'f':
            *type = CON_FALSE;
            *length = on_read_single(data, "false");
            break;
        case 'n':
            *type = CON_NULL;
            *length = on_read_single(data, "null");
            break;
        case '{':
            *type = CON_OBJECT;
            break;
        case '[':
            *type = CON_ARRAY;
            break;
        default:
            return on_read_number(data, length, type);
    }

    return NULL;
}

int on_read_composite(Object *o, const char *data, enum ValueType type);

int on_read_array(Object *o, const char *data) {
    int offset = 1;
    int off = 0;
    char *key = NULL;
    void *value = NULL;
    enum ValueType type;
    int index = 0;
    Object *x = NULL;

    while(data[offset] != ']') {
        offset += on_whitespace(data + offset);
        value = on_read_value(data + offset, &off, &type);
        if(off < 0) return -1;
        offset += off;
        on_add(o, NULL, value, type);

        if(type == CON_OBJECT || type == CON_ARRAY) {
            x = on_get_array(o, index);
            off = on_read_composite(x, data + offset, type);
            if(off < 0) return -1;
            offset += off;
        }

        offset += on_whitespace(data + offset);
        if(data[offset] == ',') offset++;
        index++;
    }

    return offset + 1;
}

int on_read_composite(Object *o, const char *data, enum ValueType type) {
    int offset = 0;

    if(type == CON_OBJECT) offset = on_loads(o, data);
    else if(type == CON_ARRAY) offset = on_read_array(o, data);
    else return -1;

    return offset;
}

int on_loads(Object *o, const char *data) {
    long offset = 1;
    int off = 0;

    if(data[0] != '{') return -1;

    offset += on_whitespace(data + offset);

    while(data[offset] != '}') {
        char *key = NULL;
        void *value = NULL;
        enum ValueType type;

        key = on_read_string(data + offset, &off);
        if(off < 0) return -1;
        offset += off;

        offset += on_whitespace(data + offset);

        if(data[offset] != ':') return -1;
        offset++;

        offset += on_whitespace(data + offset);

        value = on_read_value(data + offset, &off, &type);
        if(off < 0) return -1;
        offset += off;

        on_add(o, key, value, type);

        if(type == CON_OBJECT || type == CON_ARRAY) {
            Object *x = on_get(o, key);
            off = on_read_composite(x, data + offset, type);
            if(off < 0) return -1;
            offset += off;
        }

        free(key);
        free(value);

        offset += on_whitespace(data + offset);

        if(data[offset] == ',') {
            offset++;
            offset += on_whitespace(data + offset);
        }
    }

    return offset + 1;
}

Object *on_load(const char* filename) {
    Object *o = on_create();

    FILE *f = fopen(filename, "r");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = malloc(size + 1);
    fread(data, size, 1, f);
    data[size] = '\0';
    fclose(f);

    int status = on_loads(o, data);
    free(data);

    return o;
}
