
# CON

C Object Notation, aka JSON support in C


## Run Locally

Clone the project

```bash
  git clone https://github.com/Friedchicken-42/con.git con-master
```

Go to the project directory

```bash
  cd con-master
```

Compile (will probably add a Makefile)

```bash
  gcc -o on src/*.c main.c
```

Run the program

```bash
  ./on <filename>
```


## Usage/Examples

```c
#include "on.h"

Object *o = on_create_on();
on_add(o, "question", "What is the answer to the ultimate question?", CON_STRING);
int x = 42;
on_add(o, "answer", &x, CON_INTEGER);

char *str = on_dumps(o);
printf("%s\n", str);
```


## Testing

Compile with the testing library
```bash
  gcc -o test src/*.c test.c
```

Run the tests
```bash
  ./on
```


## TODO

- [x] add dump to file
- [ ] fix error checking
