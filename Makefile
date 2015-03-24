
SOURCES = main.c msgSend.c msgBuild.c msgXML.c server.c spRec.c cJSON.c strsub.c config.c jconfig.c
OBJECTS = $(SOURCES:.c=.o)

CC = gcc
#CC = bfin-linux-uclibc-gcc

CFLAGS = -Wall -ggdb -D_GNU_SOURCE 
LDFLAGS = -lcurl -levent -lpthread -lm -lexpat

#%.o : %.c
#	$(CC) $(CFLAGS) -c $<

all:	main

main: 	$(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o main $(LDFLAGS)

msgSend:  msgSend.o msgBuild.o spRec.o cJSON.o
	$(CC) $(CFLAGS) msgSend.o msgBuild.o spRec.o cJSON.o -o msgSend $(LDFLAGS)

server:	server.o
	$(CC) $(CFLAGS) server.o  -o server -levent

spRec:	spRec.o cJSON.o
	$(CC) $(CFLAGS) spRec.o cJSON.o  -o spRec -lm

clean:
	rm -f *.o main msgSend server spRec


dep:
	$(CC) -M $(SOURCES) $(CFLAGS)  > .depend

-include .depend
