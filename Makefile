CC := gcc
CCFLAGS := -Wall

on: src/on.c src/on.h
	$(CC) $(CCFLAGS) -c -o obj/on.o src/on.c
	ar ruv lib/on.a obj/on.o
	ranlib lib/on.a

json: on src/json/*
	$(CC) $(CCFLAGS) -c -o obj/json_loads.o src/json/json_loads.c
	$(CC) $(CCFLAGS) -c -o obj/json_dumps.o src/json/json_dumps.c
	ar ruv lib/json.a obj/json_loads.o obj/json_dumps.o lib/on.a
	ranlib lib/json.a

main: json on
	$(CC) $(CCFLAGS) -o main main.c lib/*

test: json on
	$(CC) $(CCFLAGS) -o test tests/test.c lib/*
