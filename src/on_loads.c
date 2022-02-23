#include "on.h"
#include <stdio.h>
#include <stdlib.h>

enum ConError {
    CONERR_OK = 0,
    CONERR_MISSING_QUOTATION,
    CONERR_MISSING_COLON,
    CONERR_MISMATCHING_WORD,
    CONERR_NAN,
    CONERR_EXTRA_COMMA,
    CONERR_NOT_AN_OBJECT,
    CONERR_NOT_AN_ARRAY,
    CONERR_NOT_COMPOSITE,
    CONERR_EXTRA_CHARACTERS,
};

int on_loads_on(Object *o, const char *data, int *offset);
int on_read_composite(Object *o, const char *data, int *offset, enum ValueType type);

int on_whitespace(const char *data, int *offset) {
    while(1) {
        switch(data[*offset]) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                (*offset)++;
                break;
            default:
                return CONERR_OK;
        }
    }
}

int on_read_string(const char *data, int *offset, char **strp) {
    if (data[*offset] != '"') return CONERR_MISSING_QUOTATION;

    (*offset)++;
    int length = 0;
    const char *string = data + *offset;

    while(string[length] != '"') {
        if(string[length] == '\\') {
            length++;
            if(string[length] == 'u') length += 3;
        }
        length++;
    }

    *strp = (char*)malloc(length + 1);
    for(int i = 0; i < length; i++) {
        (*strp)[i] = string[i];
    }
    (*strp)[length] = '\0';

    *offset += length + 1;

    return CONERR_OK;
}

int on_read_single(const char *data, int* offset, const char *string) {
    for(int i = 0; string[i] != '\0'; i++, (*offset)++) {
        if (data[*offset] != string[i]) return CONERR_MISMATCHING_WORD;
    }

    return CONERR_OK;
}

char on_is_digit(const char chr) {
    return chr >= '0' && chr <= '9';
}

int on_read_integer(const char *data, int *offset, int *number) {
    char sign = 1;
    
    if(data[*offset] == '-') {
        sign = -1;
        (*offset)++;
    }
    

    if(data[*offset] == '0') {
        (*offset)++;
    } else if (on_is_digit(data[*offset])) {
        while(on_is_digit(data[*offset])) {
            *number = (*number * 10) + (int)(data[*offset] - '0');
            (*offset)++;
        }
    } else {
        return CONERR_NAN;
    }

    *number = sign * *number;

    return CONERR_OK;
}

int on_read_fraction(const char *data, int *offset, int *fraction) {
    if(data[*offset] == '.') {
        (*offset)++;
        while(on_is_digit(data[*offset])) {
            *fraction = (*fraction * 10) + (int)(data[*offset] - '0');
            (*offset)++;
        }
    }

    return CONERR_OK;
}

int on_read_exponent(const char *data, int *offset, int *exponent) {
    char sign = 1;

    if(data[*offset] != 'e' && data[*offset] != 'E') return CONERR_OK;

    (*offset)++;

    if(data[*offset] == '-') sign = -1;
    else if (data[*offset] == '+') sign = 1;
    else if(on_is_digit(data[*offset])) (*offset)--;
    else return CONERR_NAN;

    (*offset)++;

    if(!on_is_digit(data[*offset])) return CONERR_NAN;

    while(on_is_digit(data[*offset])) {
        *exponent = (*exponent * 10) + (int)(data[*offset] - '0');
        (*offset)++;
    }

    *exponent = sign * *exponent;

    return CONERR_OK;
}

int on_read_number(const char *data, int *offset, enum ValueType *type, void **value) {
    int number = 0;
    int fraction = 0;
    int exponent = 0;
    int err = 0;

    err = on_read_integer(data, offset, &number);
    if(err) return err;

    err = on_read_fraction(data, offset, &fraction);
    if(err) return err;
    
    err = on_read_exponent(data, offset, &exponent);
    if(err) return err;

    if(fraction == 0 && exponent >= 0) {
        *value = (int*)malloc(sizeof(int));
        *(int*)*value = number; 
        *type = CON_INTEGER;
    } else {
        *value = (float*)malloc(sizeof(float));
        *(float*)*value = number;
        *type = CON_FLOAT;
    }

    float fr = fraction;
    if(fraction != 0) {
        while((int)fr != 0) fr /= 10;
        *(float*)*value += fr;
    }

    int abs_exp = exponent < 0 ? -exponent : exponent;
    for(int i = 0; i < abs_exp; i++) {
        if(*type == CON_INTEGER) {
            *(int*)*value *= 10;
        } else if(exponent > 0) {
            *(float*)*value *= 10;
        } else {
            *(float*)*value /= 10;
        }
    }

    return CONERR_OK;
}

int on_read_value(const char *data, int *offset, enum ValueType *type, void **value) {
    switch(data[*offset]) {
        case '"':
            *type = CON_STRING; 
            return on_read_string(data, offset, (char**)value);
        case 't':
            *type = CON_TRUE;
            return on_read_single(data, offset, "true");
        case 'f':
            *type = CON_FALSE;
            return on_read_single(data, offset, "false");
        case 'n':
            *type = CON_NULL;
            return on_read_single(data, offset, "null");
        case '{':
            *type = CON_OBJECT;
            return CONERR_OK;
        case '[':
            *type = CON_ARRAY;
            return CONERR_OK;
        default:
            return on_read_number(data, offset, type, value);
    }
}

int on_read_array(Object *o, const char *data, int *offset) {
    void *value = NULL;
    enum ValueType type = 0;
    int index = 0;
    Object *x = NULL;
    int err = 0;

    if(data[*offset] != '[') return CONERR_NOT_AN_ARRAY;
    (*offset)++;

    while(data[*offset] != ']') {
        on_whitespace(data, offset);
        err = on_read_value(data, offset, &type, &value);
        if(err) return err;

        on_add(o, NULL, value, type);
        free(value);

        on_whitespace(data, offset);

        if(type == CON_OBJECT || type == CON_ARRAY) {
            x = on_get_array(o, index);
            err = on_read_composite(x, data, offset, type);
            if(err) return err;
        }

        on_whitespace(data, offset);
        if(data[*offset] == ',') {
            (*offset)++;
            on_whitespace(data, offset);
            if(data[*offset] == ']' || data[*offset] == '}') return CONERR_EXTRA_COMMA;
        }
        index++;
    }
    
    on_whitespace(data, offset);
    (*offset)++;

    return CONERR_OK;
}

int on_read_composite(Object *o, const char *data, int *offset, enum ValueType type) {
    int err = 0;

    if(type == CON_OBJECT) err = on_loads_on(o, data, offset);
    else if(type == CON_ARRAY) err = on_read_array(o,  data, offset);
    else return CONERR_NOT_COMPOSITE;

    return err;
}

int on_loads_on(Object *o, const char *data, int *offset) {
    if(data[*offset] != '{') return CONERR_NOT_AN_OBJECT;
    (*offset)++;

    on_whitespace(data, offset);

    while(data[*offset] != '}') {
        char *key = NULL;
        void *value = NULL;
        enum ValueType type;
        int err = 0;

        err = on_read_string(data, offset, &key);
        if(err) return err;

        on_whitespace(data, offset);

        if(data[*offset] != ':') return CONERR_MISSING_COLON;
        (*offset)++;

        on_whitespace(data, offset);

        on_read_value(data, offset, &type, &value);
        if(err) return err;
        
        on_add(o, key, value, type);

        if(type == CON_OBJECT || type == CON_ARRAY) {
            Object *x = on_get(o, key);
            err = on_read_composite(x, data, offset, type);
            if(err) return err;
        }

        free(key);
        free(value);

        on_whitespace(data, offset);

        if(data[*offset] == ',') {
            (*offset)++;
            on_whitespace(data, offset);
            if(data[*offset] == ']' || data[*offset] == '}') return CONERR_EXTRA_COMMA;
        }
    }
    
    on_whitespace(data, offset);
    (*offset)++;

    return CONERR_OK;
}

Object *on_loads(const char* data) {
    Object *o = NULL;
    int offset = 0;
    int err = 0;

    if(data[0] == '{') {
        o = on_create_on();
        err = on_loads_on(o, data, &offset);
    } else if(data[0] == '[') {
        o = on_create_array();
        err = on_read_array(o, data, &offset);
    }

    if(err) {
        fprintf(stderr, "Error: %d\n", err);
        return NULL;
    }

    on_whitespace(data, &offset);

    if(data[offset] != '\0') {
        err = CONERR_EXTRA_CHARACTERS;
        fprintf(stderr, "Error: %d\n", err);
        return NULL;
    }

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
