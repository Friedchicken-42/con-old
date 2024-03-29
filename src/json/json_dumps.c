#include <stdio.h>
#include <stdlib.h>
#include "../on.h"

int json_tabs(char *buffer, int size, int tab, char is_write) {
    int offset = 0;
    for(int i = 0; i < tab; i++) offset += snprintf(is_write ? buffer + offset : NULL, size, "\t");
    return offset;
}

int json_sndumps_on(Object *o, char *buffer, int size, int tab, char is_write) {
    int offset = 0;
    Object *curr = o;

    while(curr != NULL) {
        offset += json_tabs(buffer + offset, size, tab, is_write);

        if(curr->key) offset += snprintf(is_write ? buffer + offset : NULL, size, "\"%s\": ", curr->key);

        switch(curr->type) {
            case CON_EMPTY:
                break;
            case CON_INTEGER:
                offset += snprintf(is_write ? buffer + offset : NULL, size, "%d", *(int*)curr->value);
                break;
            case CON_FLOAT:
                offset += snprintf(is_write ? buffer + offset : NULL, size, "%f", *(float*)curr->value);
                break;
            case CON_STRING:
                offset += snprintf(is_write ? buffer + offset : NULL, size, "\"%s\"", (char*)curr->value);
                break;
            case CON_TRUE:
                offset += snprintf(is_write ? buffer + offset : NULL, size, "true");
                break;
            case CON_FALSE:
                offset += snprintf(is_write ? buffer + offset : NULL, size, "false");
                break;
            case CON_NULL:
                offset += snprintf(is_write ? buffer + offset : NULL, size, "null");
                break;
            case CON_ARRAY:
                offset += snprintf(is_write ? buffer + offset : NULL, size, "[\n");
                offset += json_sndumps_on(curr->value, buffer + offset, size, tab + 1, is_write);
                offset += json_tabs(buffer + offset, size, tab, is_write);
                offset += snprintf(is_write ? buffer + offset : NULL, size, "]");
                break;
            case CON_OBJECT:
                offset += snprintf(is_write ? buffer + offset : NULL, size, "{\n");
                offset += json_sndumps_on(curr->value, buffer + offset, size, tab + 1, is_write);
                offset += json_tabs(buffer + offset, size, tab, is_write);
                offset += snprintf(is_write ? buffer + offset : NULL, size, "}");
                break;
            default:
                break;
        }

        curr = curr->next;
        if(curr) offset += snprintf(is_write ? buffer + offset : NULL, size, ",");
        offset += snprintf(is_write ? buffer + offset : NULL, size, "\n");
    }
    
    return offset;
}

int json_sndumps(Object *o, char *buffer, int size, char is_write) {
    if(o == NULL) return snprintf(is_write ? buffer : NULL, size, "{}");
    int offset = 0;
    offset += snprintf(is_write ? buffer + offset : NULL, size, o->parent == CON_OBJECT ? "{\n" : "[\n");
    offset += json_sndumps_on(o, buffer + offset, size, 1, is_write);
    offset += snprintf(is_write ? buffer + offset : NULL, size, o->parent == CON_OBJECT ? "}" : "]");
    return offset;
}

char *json_dumps(Object *o) {
    int length = json_sndumps(o, NULL, 0, 0);
    char *string = (char*)malloc(length + 1);

    json_sndumps(o, string, length, 1);
    string[length] = '\0';

    return string; 
}

void json_dump(Object *o, const char* filename) {
    FILE *f = fopen(filename, "w");
    char *string = json_dumps(o);
    fputs(string, f);
    
    free(string);
    fclose(f);
}
