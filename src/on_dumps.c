#include <stdio.h>
#include "on.h"

void on_print_tabbed(Object *o, int tab) {
    Object *curr = o;

    while(curr != NULL) {
        for(int i = 0; i < tab; i++) printf("\t");
        if(curr->key) printf("\"%s\": ", curr->key);

        switch(curr->type) {
            case CON_EMPTY:
                break;
            case CON_INTEGER:
                printf("%d", *(int*)curr->value);
                break;
            case CON_FLOAT:
                printf("%f", *(float*)curr->value);
                break;
            case CON_STRING:
                printf("\"%s\"", (char*)curr->value);
                break;
            case CON_TRUE:
                printf("true");
                break;
            case CON_FALSE:
                printf("false");
                break;
            case CON_NULL:
                printf("null");
                break;
            case CON_ARRAY:
                printf("[\n");
                on_print_tabbed(curr->value, tab + 1);
                for(int i = 0; i < tab; i++) printf("\t");
                printf("]");
                break;
            case CON_OBJECT:
                printf("{\n");
                on_print_tabbed(curr->value, tab + 1);
                for(int i = 0; i < tab; i++) printf("\t");
                printf("}");
                break;
            default:
                printf("this value is not supported for printing");
                break;
        }

        curr = curr->next;
        if(curr) printf(",");
        printf("\n");
    }
}

void on_print(Object *o) {
    printf("{\n");
    on_print_tabbed(o, 1);
    printf("}\n");
}
