CFLAGS = -std=c99 -Wall -pedantic
all: j1

j1: j1.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: run clean

run: j1
	./j1

clean:
	rm -f j1
