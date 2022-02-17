#include "on.h"
#include <stdio.h>
#include <stdlib.h>

int on_loads_on(Object *o, const char *data);
int on_read_composite(Object *o, const char *data, enum ValueType type);

int on_whitespace(const char *data) {
    int length = 0;
    while(1) {
        switch(data[length]) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                length++;
                break;
            default:
                return length;
        }
    }
}

char *on_read_string(const char *data, int *length) {
    if (data[0] != '"') {
        return NULL;
        *length = -1;
    }
    *length = 1;

    while(data[*length] != '"') {
        if(data[*length] == '\\') {
            *length += 1;
            if(data[*length] == 'u') *length += 3;
        }
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

char on_is_digit(const char chr) {
    return chr >= '0' && chr <= '9';
}

int on_read_integer(const char *data, int *size) {
    int number = 0;
    char sign = 1;
    
    if(data[0] == '-') {
        sign = -1;
        *size += 1;
    }
    

    if(data[*size] == '0') {
        *size += 1;
    } else if (on_is_digit(data[*size])) {
        while(on_is_digit(data[*size])) {
            number = (number * 10) + (int)(data[*size] - '0');
            *size += 1;
        }
    } else {
        *size = -1;
    }

    return sign * number;
}

int on_read_fraction(const char *data, int *length, int *size) {
    int fraction = 0;

    if(data[*size] == '.') {
        *size += 1;
        while(on_is_digit(data[*size])) {
            fraction = (fraction * 10) + (int)(data[*size] - '0');
            *size += 1;
            *length += 1; 
        }
    }

    return fraction;
}

int on_read_exponent(const char *data, int *size) {
    int exponent = 0;
    char sign = 1;

    if(data[*size] == 'e' || data[*size] == 'E') {
        *size += 1;
        if(data[*size] == '-') {
            sign = -1;
            *size += 1;
        } else if (data[*size] == '+') {
            *size += 1;
        } else if(!on_is_digit(data[*size])) {
            *size = -1;
            return 0;
        }
        if(!on_is_digit(data[*size])) {
            *size = -1;
            return 0;
        }
        
        while(on_is_digit(data[*size])) {
            exponent = (exponent * 10) + (int)(data[*size] - '0');
            *size += 1;
        }
    }

    return sign * exponent;
}

void *on_read_number(const char *data, int *length, enum ValueType *type) {
    int size = 0;
    int number = 0;
    int fraction = 0;
    int fraction_size = 0;
    int exponent = 0;

    number = on_read_integer(data, &size);
    if(size < 0) {
        *length = -1;
        return NULL;
    }

    fraction = on_read_fraction(data, &fraction_size, &size);
    
    exponent = on_read_exponent(data, &size);
    if(size < 0) {
        *length = -1;
        return NULL;
    }

    void *value = NULL;

    if(fraction == 0 && exponent >= 0) {
        value = (int*)malloc(sizeof(int));
        *(int*)value = number; 
        *type = CON_INTEGER;
    } else {
        value = (float*)malloc(sizeof(float));
        *(float*)value = number;
        *type = CON_FLOAT;
    }

    float fr = fraction;
    if(fraction != 0) {
        for(int i = 0; i < fraction_size; i++) fr /= 10;
        *(float*)value += fr;
    }

    int abs_exp = exponent < 0 ? -exponent : exponent;
    for(int i = 0; i < abs_exp; i++) {
        if(*type == CON_INTEGER) {
            *(int*)value *= 10;
        } else if(exponent > 0) {
            *(float*)value *= 10;
        } else {
            *(float*)value /= 10;
        }
    }

    *length = size;
    return value;
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

int on_read_array(Object *o, const char *data) {
    int offset = 1;
    int off = 0;
    void *value = NULL;
    enum ValueType type = 0;
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
        if(data[offset] == ',') {
            offset++;
            offset += on_whitespace(data + offset);
            if(data[offset] == ']' || data[offset] == '}') return -1;
        }
        index++;
    }
    
    offset += on_whitespace(data + offset);

    return offset + 1;
}

int on_read_composite(Object *o, const char *data, enum ValueType type) {
    int offset = 0;

    if(type == CON_OBJECT) offset = on_loads_on(o, data);
    else if(type == CON_ARRAY) offset = on_read_array(o, data);
    else return -1;

    return offset;
}

int on_loads_on(Object *o, const char *data) {
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
            if(data[offset] == ']' || data[offset] == '}') return -1;
        }
    }
    
    offset += on_whitespace(data + offset);

    return offset + 1;
}

Object *on_loads(const char* data) {
    Object *o = on_create();
    int status = -1;

    if(data[0] == '{') status = on_loads_on(o, data);
    else if(data[0] == '[') status = on_read_array(o, data);

    if(status == -1) return NULL;
    status += on_whitespace(data + status);

    if(data[status] != '\0') return NULL;

    return o;
}

Object *on_load(const char* filename) {
    FILE *f = fopen(filename, "rb");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = NULL;
    data = malloc(size + 1);
    fread(data, size, 1, f);
    data[size] = '\0';
    fclose(f);

    Object *o = on_loads(data);
    free(data);

    return o;
}
