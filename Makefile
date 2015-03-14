
SOURCES = curlTest.c sp_msg.c
OBJECTS = $(SOURCES:.c=.o)

CC = gcc
#CC = bfin-linux-uclibc-gcc

CFLAGS = -Wall  -D_GNU_SOURCE -DTEST
LDFLAGS = -lcurl -lpthread

%.o : %.c
	$(CC) $(CFLAGS) -c $<

all:	msgSend server spRec

msgSend:  msgSend.o msgBuild.o
	$(CC) $(CFLAGS) msgSend.o msgBuild.o -o msgSend -lcurl -lpthread

server:	server.o
	$(CC) $(CFLAGS) server.o  -o server -levent

spRec:	spRec.o cJSON.o
	$(CC) $(CFLAGS) spRec.o cJSON.o  -o spRec -lm

clean:
	rm -f *.o msgSend server spRec


