CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Werror -D_XOPEN_SOURCE=700

.PHONY: all clean

all: clash

clean: 
	rm -f clash clash.o

clash: clash.o
	$(CC) $(CFLAGS) -o clash clash.o

clash.o: clash.c plist.c
	$(CC) -c clash.c plist.c

