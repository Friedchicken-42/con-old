#include <stdio.h>
#include <stdlib.h>
#include "on.h"

int on_tabs(char *buffer, int size, int tab) {
    int offset = 0;
    for(int i = 0; i < tab; i++) offset += snprintf(buffer + offset, size, "\t");
    return offset;
}

int on_sndumps_on(Object *o, char *buffer, int size, int tab) {
    int offset = 0;
    Object *curr = o;

    while(curr != NULL) {
        offset += on_tabs(buffer + offset, size, tab);

        if(curr->key) offset += snprintf(buffer + offset, size, "\"%s\": ", curr->key);

        switch(curr->type) {
            case CON_EMPTY:
                break;
            case CON_INTEGER:
                offset += snprintf(buffer + offset, size, "%d", *(int*)curr->value);
                break;
            case CON_FLOAT:
                offset += snprintf(buffer + offset, size, "%f", *(float*)curr->value);
                break;
            case CON_STRING:
                offset += snprintf(buffer + offset, size, "\"%s\"", (char*)curr->value);
                break;
            case CON_TRUE:
                offset += snprintf(buffer + offset, size, "true");
                break;
            case CON_FALSE:
                offset += snprintf(buffer + offset, size, "false");
                break;
            case CON_NULL:
                offset += snprintf(buffer + offset, size, "null");
                break;
            case CON_ARRAY:
                offset += snprintf(buffer + offset, size, "[\n");
                offset += on_sndumps_on(curr->value, buffer + offset, size, tab + 1);
                offset += on_tabs(buffer + offset, size, tab);
                offset += snprintf(buffer + offset, size, "]");
                break;
            case CON_OBJECT:
                offset += snprintf(buffer + offset, size, "{\n");
                offset += on_sndumps_on(curr->value, buffer + offset, size, tab + 1);
                offset += on_tabs(buffer + offset, size, tab);
                offset += snprintf(buffer + offset, size, "}");
                break;
            default:
                break;
        }

        curr = curr->next;
        if(curr) offset += snprintf(buffer + offset, size, ",");
        offset += snprintf(buffer + offset, size, "\n");
    }
    
    return offset;
}

int on_sndumps(Object *o, char *buffer, int size) {
    if(o == NULL) return snprintf(buffer, size, "{}");
    int offset = 0;
    offset += snprintf(buffer + offset, size, o->key ? "{\n" : "[\n");
    offset += on_sndumps_on(o, buffer + offset, size, 1);
    offset += snprintf(buffer + offset, size, o->key ? "}\n" : "]\n");
    return offset;
}

char *on_dumps(Object *o) {
    int length = on_sndumps(o, NULL, 0);
    char *string = (char*)malloc(length + 1);

    on_sndumps(o, string, length);
    string[length] = '\0';

    return string; 
}

void on_dump(Object *o, const char* filename) {
    FILE *f = fopen(filename, "w");
    char *string = on_dumps(o);
    fputs(string, f);
    
    free(string);
    fclose(f);
}
