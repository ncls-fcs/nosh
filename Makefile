CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Werror -D_XOPEN_SOURCE=700

.PHONY: all clean

all: clash

clean: 
	rm -f clash clash.o plist.o

clash: clash.o plist.o
	$(CC) $(CFLAGS) -o clash clash.o plist.o

clash.o: clash.c
	$(CC) $(CFLAGS) -c clash.c
##I----> #--------------------------------------------------------------------#
         # #K plist.h fehlt als Abhängigkeit für clash.o (-0.5)               #
         #-------------------------------------------------------------------#/

plist.o: plist.c plist.h
	$(CC) $(CFLAGS) -c plist.c



##P----> #--------------------------------------------------------------------#
         # Punktabzug in dieser Datei: 0.5 Punkte                             #
         #-------------------------------------------------------------------#/
