#include "../src/json/json.h"
#include <stdio.h>

#define true 1
#define false 0

int check(char *filename) {
    return json_load(filename) == NULL ? false : true;
}

void test(char *folder, char *name, int length, int expected, int *pass, int *fail) {
    char filename[32];
    int pass_local = 0;
    int fail_local = 0;

    printf("%s\n", name);
    for(int i = 1; i <= length; i++) {
        snprintf(filename, 32, "tests/%s/%s%02d.json", folder, name, i);
        if(check(filename) == expected) {
            printf("\t[ Pass ] %s\n", filename);
            pass_local++;
        } else {
            printf("\t[ Fail ] %s\n", filename);
            fail_local++;
        }
    }
    printf("%d passed, %d failed\n\n", pass_local, fail_local);

    *pass += pass_local;
    *fail += fail_local;
}

int main() {
    int pass = 0;
    int fail = 0;
    test("json", "pass", 26, true, &pass, &fail);
    test("json", "fail", 26, false, &pass, &fail);
    test("json", "complete", 1, true, &pass, &fail);
    
    printf("Total: %d passed, %d failed\n", pass, fail);
}
