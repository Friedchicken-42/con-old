#include "../on.h"
#include <stdio.h>
#include <stdlib.h>

enum JsonError {
    JSONERR_OK = 0,
    JSONERR_MISSING_QUOTATION,
    JSONERR_MISSING_COLON,
    JSONERR_MISMATCHING_WORD,
    JSONERR_NAN,
    JSONERR_EXTRA_COMMA,
    JSONERR_NOT_AN_OBJECT,
    JSONERR_NOT_AN_ARRAY,
    JSONERR_NOT_COMPOSITE,
    JSONERR_EXTRA_CHARACTERS,
};

void json_print_error_message(const char *data, int offset, enum JsonError err) {
    char *convert[] = {
        [JSONERR_OK] = "",
        [JSONERR_MISSING_QUOTATION] = "Missing \"\"\" in string",
        [JSONERR_MISSING_COLON] = "Missing \":\" after key",
        [JSONERR_MISMATCHING_WORD] = "Wrong single word",
        [JSONERR_NAN] = "Not a Number",
        [JSONERR_EXTRA_COMMA] = "Extra comma before closing",
        [JSONERR_NOT_AN_OBJECT] = "Expected Object",
        [JSONERR_NOT_AN_ARRAY] = "Expected Array",
        [JSONERR_NOT_COMPOSITE] = "Expected Object or Array",
        [JSONERR_EXTRA_CHARACTERS] = "Extra character after end of object",
    };
    
    fprintf(stderr, "Error %02d: \"%s\" on position %d: \"%c\"\n", err, convert[err], offset, data[offset]);
}

int json_loads_on(Object *o, const char *data, int *offset);
int json_read_composite(Object *o, const char *data, int *offset, enum ValueType type);

int json_whitespace(const char *data, int *offset) {
    while(1) {
        switch(data[*offset]) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                (*offset)++;
                break;
            default:
                return JSONERR_OK;
        }
    }
}

int json_read_string(const char *data, int *offset, char **strp) {
    if (data[*offset] != '"') return JSONERR_MISSING_QUOTATION;

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

    return JSONERR_OK;
}

int json_read_single(const char *data, int* offset, const char *string) {
    for(int i = 0; string[i] != '\0'; i++, (*offset)++) {
        if (data[*offset] != string[i]) return JSONERR_MISMATCHING_WORD;
    }

    return JSONERR_OK;
}

char json_is_digit(const char chr) {
    return chr >= '0' && chr <= '9';
}

int json_read_integer(const char *data, int *offset, int *number) {
    char sign = 1;
    
    if(data[*offset] == '-') {
        sign = -1;
        (*offset)++;
    }
    

    if(data[*offset] == '0') {
        (*offset)++;
    } else if (json_is_digit(data[*offset])) {
        while(json_is_digit(data[*offset])) {
            *number = (*number * 10) + (int)(data[*offset] - '0');
            (*offset)++;
        }
    } else {
        return JSONERR_NAN;
    }

    *number = sign * *number;

    return JSONERR_OK;
}

int json_read_fraction(const char *data, int *offset, int *fraction) {
    if(data[*offset] == '.') {
        (*offset)++;
        while(json_is_digit(data[*offset])) {
            *fraction = (*fraction * 10) + (int)(data[*offset] - '0');
            (*offset)++;
        }
    }

    return JSONERR_OK;
}

int json_read_exponent(const char *data, int *offset, int *exponent) {
    char sign = 1;

    if(data[*offset] != 'e' && data[*offset] != 'E') return JSONERR_OK;

    (*offset)++;

    if(data[*offset] == '-') sign = -1;
    else if (data[*offset] == '+') sign = 1;
    else if(json_is_digit(data[*offset])) (*offset)--;
    else return JSONERR_NAN;

    (*offset)++;

    if(!json_is_digit(data[*offset])) return JSONERR_NAN;

    while(json_is_digit(data[*offset])) {
        *exponent = (*exponent * 10) + (int)(data[*offset] - '0');
        (*offset)++;
    }

    *exponent = sign * *exponent;

    return JSONERR_OK;
}

int json_read_number(const char *data, int *offset, enum ValueType *type, void **value) {
    int number = 0;
    int fraction = 0;
    int exponent = 0;
    int err = 0;

    err = json_read_integer(data, offset, &number);
    if(err) return err;

    err = json_read_fraction(data, offset, &fraction);
    if(err) return err;
    
    err = json_read_exponent(data, offset, &exponent);
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

    return JSONERR_OK;
}

int json_read_value(const char *data, int *offset, enum ValueType *type, void **value) {
    switch(data[*offset]) {
        case '"':
            *type = CON_STRING; 
            return json_read_string(data, offset, (char**)value);
        case 't':
            *type = CON_TRUE;
            return json_read_single(data, offset, "true");
        case 'f':
            *type = CON_FALSE;
            return json_read_single(data, offset, "false");
        case 'n':
            *type = CON_NULL;
            return json_read_single(data, offset, "null");
        case '{':
            *type = CON_OBJECT;
            return JSONERR_OK;
        case '[':
            *type = CON_ARRAY;
            return JSONERR_OK;
        default:
            return json_read_number(data, offset, type, value);
    }
}

int json_read_array(Object *o, const char *data, int *offset) {
    enum ValueType type = 0;
    int index = 0;
    Object *x = NULL;
    int err = 0;

    if(data[*offset] != '[') return JSONERR_NOT_AN_ARRAY;
    (*offset)++;

    json_whitespace(data, offset);

    while(data[*offset] != ']') {
        void *value = NULL;
        json_whitespace(data, offset);
        err = json_read_value(data, offset, &type, &value);
        if(err) return err;

        on_add(o, NULL, value, type);
        free(value);

        json_whitespace(data, offset);

        if(type == CON_OBJECT || type == CON_ARRAY) {
            x = on_get_array(o, index);
            err = json_read_composite(x, data, offset, type);
            if(err) return err;
        }

        json_whitespace(data, offset);
        if(data[*offset] == ',') {
            (*offset)++;
            json_whitespace(data, offset);
            if(data[*offset] == ']' || data[*offset] == '}') return JSONERR_EXTRA_COMMA;
        }
        index++;
    }
    
    json_whitespace(data, offset);
    (*offset)++;

    return JSONERR_OK;
}

int json_read_composite(Object *o, const char *data, int *offset, enum ValueType type) {
    int err = 0;

    if(type == CON_OBJECT) err = json_loads_on(o, data, offset);
    else if(type == CON_ARRAY) err = json_read_array(o,  data, offset);
    else return JSONERR_NOT_COMPOSITE;

    return err;
}

int json_loads_on(Object *o, const char *data, int *offset) {
    if(data[*offset] != '{') return JSONERR_NOT_AN_OBJECT;
    (*offset)++;

    json_whitespace(data, offset);

    while(data[*offset] != '}') {
        char *key = NULL;
        void *value = NULL;
        enum ValueType type;
        int err = 0;

        err = json_read_string(data, offset, &key);
        if(err) return err;

        json_whitespace(data, offset);

        if(data[*offset] != ':') return JSONERR_MISSING_COLON;
        (*offset)++;

        json_whitespace(data, offset);

        json_read_value(data, offset, &type, &value);
        if(err) return err;
        
        on_add(o, key, value, type);

        if(type == CON_OBJECT || type == CON_ARRAY) {
            Object *x = on_get(o, key);
            err = json_read_composite(x, data, offset, type);
            if(err) return err;
        }

        free(key);
        free(value);

        json_whitespace(data, offset);

        if(data[*offset] == ',') {
            (*offset)++;
            json_whitespace(data, offset);
            if(data[*offset] == ']' || data[*offset] == '}') return JSONERR_EXTRA_COMMA;
        }
    }
    
    json_whitespace(data, offset);
    (*offset)++;

    return JSONERR_OK;
}

Object *json_loads(const char* data) {
    Object *o = NULL;
    int offset = 0;
    int err = 0;

    if(data[0] == '{') {
        o = on_create_on();
        err = json_loads_on(o, data, &offset);
    } else if(data[0] == '[') {
        o = on_create_array();
        err = json_read_array(o, data, &offset);
    }

    if(err) {
        json_print_error_message(data, offset, err);
        return NULL;
    }

    json_whitespace(data, &offset);

    if(data[offset] != '\0') {
        err = JSONERR_EXTRA_CHARACTERS;
        json_print_error_message(data, offset, err);
        return NULL;
    }

    return o;
}

Object *json_load(const char* filename) {
    FILE *f = fopen(filename, "rb");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = NULL;
    data = malloc(size + 1);
    fread(data, size, 1, f);
    data[size] = '\0';
    fclose(f);

    Object *o = json_loads(data);
    free(data);

    return o;
}
