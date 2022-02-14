#include <stdio.h>
#include <stdlib.h>
#include "on.h"

int on_tabs(char *buffer, int size, int tab) {
    for(int i = 0; i < tab; i++) snprintf(buffer, size, "%s\t", buffer);
    return tab;
}

int on_sndumps_on(Object *o, char *buffer, int size, int tab) {
    Object *curr = o;
    int length = 0;

    while(curr != NULL) {
        length += on_tabs(buffer, size, tab);

        if(curr->key) length += snprintf(buffer, size, "%s\"%s\": ", buffer, curr->key);

        switch(curr->type) {
            case CON_EMPTY:
                break;
            case CON_INTEGER:
                length += snprintf(buffer, size, "%s%d", buffer, *(int*)curr->value);
                break;
            case CON_FLOAT:
                length += snprintf(buffer, size, "%s%f", buffer, *(float*)curr->value);
                break;
            case CON_STRING:
                length += snprintf(buffer, size, "%s\"%s\"", buffer, (char*)curr->value);
                break;
            case CON_TRUE:
                length += snprintf(buffer, size, "%strue", buffer);
                break;
            case CON_FALSE:
                length += snprintf(buffer, size, "%sfalse", buffer);
                break;
            case CON_NULL:
                length += snprintf(buffer, size, "%snull", buffer);
                break;
            case CON_ARRAY:
                length += snprintf(buffer, size, "%s[\n", buffer);
                length += on_sndumps_on(curr->value, buffer, size, tab + 1);
                length += on_tabs(buffer, size, tab);
                length += snprintf(buffer, size, "%s]", buffer);
                break;
            case CON_OBJECT:
                length += snprintf(buffer, size, "%s{\n", buffer);
                length += on_sndumps_on(curr->value, buffer, size, tab + 1);
                length += on_tabs(buffer, size, tab);
                length += snprintf(buffer, size, "%s}", buffer);
                break;
            default:
                break;
        }

        curr = curr->next;
        if(curr) length += snprintf(buffer, size, "%s,", buffer);
        length += snprintf(buffer, size, "%s\n", buffer);
    }
    return length;
}

int on_sndumps(Object *o, char *buffer, int size) {
    int length = 0;
    length += snprintf(buffer, size, "{\n");
    length += on_sndumps_on(o, buffer, size, 1);
    length += snprintf(buffer, size, "%s}\n", buffer);
    return length;
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
