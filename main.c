#include "src/on.h"
#include "src/json/json.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if(argc != 2) {
        printf("Use <./on> <filename>\n");
        return -1;
    }

    Object *o = json_load(argv[1]);
    if(o == NULL) {
        printf("Error  %s\n", argv[1]);
        return 1;
    }

    printf("Loaded %s\n", argv[1]);
    char *str = json_dumps(o);
    printf("%s", str);

    free(str);
    on_clean(o);

    return 0;
}
