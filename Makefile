CC ?= gcc
CFLAGS = -g -Wall -Wextra -std=gnu17

main:
	$(CC) $(CFLAGS) src/main.c -o src/main -lpthread -ldl -lz

clean:
	rm src/main
