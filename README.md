
# CON

C Object Notation, aka C support for JSON and other configuration files


## Run Locally

Clone the project

```bash
  git clone https://github.com/Friedchicken-42/con.git con-master
```

Go to the project directory

```bash
  cd con-master
```

Compile

```bash
  make main
```

Run the program

```bash
  ./main <filename>
```


## Usage/Examples

```c
#include "on.h"
#include "json/json.h"

Object *o = json_create_on();
on_add(o, "question", "What is the answer to the ultimate question?", CON_STRING);
int x = 42;
on_add(o, "answer", &x, CON_INTEGER);

char *str = json_dumps(o);
printf("%s\n", str);
```


## Testing

Compile with the testing library
```bash
  make test
```

Run the tests
```bash
  ./test
```


## TODO

- [x] add dump to file
- [x] fix error checking
- [ ] better makefile
- [ ] add parser for yaml/toml/ini
