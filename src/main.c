#include "on.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("Use <./on> <filename>\n");
        return -1;
    }

    Object *o = on_load(argv[1]);
    on_print(o);
}
