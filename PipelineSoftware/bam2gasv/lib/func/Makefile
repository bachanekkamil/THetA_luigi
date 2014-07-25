
CFLAGS=-Wall -g
LDFLAGS=

.PHONY: test

all: bin/list bin/buffer bin/hash bin/array bin/heap bin/fn bin/str

test: all
	./bin/list
	./bin/buffer
	./bin/hash
	./bin/array
	./bin/heap
	./bin/fn
	./bin/str

bin/list: src/list.c test/list.c src/list.h src/fn.h src/fn.c
	mkdir -p bin
	$(CC) $(filter %.c, $^) -o bin/list -Isrc $(CFLAGS) $(LDFLAGS)

bin/array: src/array.c test/array.c src/array.h
	mkdir -p bin
	$(CC) $(filter %.c, $^) -o bin/array -Isrc $(CFLAGS) $(LDFLAGS)

bin/heap: src/heap.c test/heap.c src/heap.h src/array.c
	mkdir -p bin
	$(CC) $(filter %.c, $^) -o bin/heap -Isrc $(CFLAGS) $(LDFLAGS)

bin/fn: src/fn.c test/fn.c src/fn.h src/list.c src/list.h
	mkdir -p bin
	$(CC) $(filter %.c, $^) -o bin/fn -Isrc $(CFLAGS) $(LDFLAGS)

bin/buffer: src/buffer.c test/buffer.c src/buffer.h
	mkdir -p bin
	$(CC) $(filter %.c, $^) -o bin/buffer -Isrc $(CFLAGS) $(LDFLAGS)

bin/hash: src/hash.c test/hash.c src/hash.h src/list.c src/list.h src/fn.h src/fn.c
	mkdir -p bin
	$(CC) $(filter %.c, $^) -o bin/hash -Isrc $(CFLAGS) $(LDFLAGS)

bin/str: src/str.c test/str.c src/str.h 
	mkdir -p bin
	$(CC) $(filter %.c, $^) -o bin/str -Isrc $(CFLAGS) $(LDFLAGS)
