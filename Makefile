
SOURCES = main.c startup.c plugins.c msgSend.c msgBuild.c msgXML.c msgQueue.c server.c spRec.c \
	cJSON.c strsub.c config.c jconfig.c logging.c queues.c alarms.c
OBJECTS = $(SOURCES:.c=.o)

CC = gcc
#CC = bfin-linux-uclibc-gcc

#If the following is defined, build the plug-in version of the code
PLUGIN = 1

ifdef PLUGIN
TARGET = main_plugin
CFLAGS = -Wall -ggdb -D_GNU_SOURCE -DPLUGIN
PFLAGS = -shared -fPIC -export-dynamic
else
TARGET = main
CFLAGS = -Wall -ggdb -D_GNU_SOURCE
endif
LDFLAGS = -lcurl -levent -lexpat -lm -lpthread -ldl


%.o : %.c
	$(CC) $(CFLAGS) $(PFLAGS) -c $<

all:	$(TARGET)

main_plugin: $(OBJECTS) plugins.o
	@echo "CREATING PLUGIN VERSION"
	$(CC) $(CFLAGS) $(PFLAGS) $(OBJECTS)  -o sp8440.so $(LDFLAGS)
	$(CC) $(CFLAGS)  main.o plugins.o  -o main_plugin $(LDFLAGS) -ldl

main: 	$(OBJECTS)
	@echo "CREATING STANDALONE VERSION"
	$(CC) $(CFLAGS1) $(OBJECTS) -o main $(LDFLAGS)

msgSend:  msgSend.o msgBuild.o spRec.o cJSON.o
	$(CC) $(CFLAGS) msgSend.o msgBuild.o spRec.o cJSON.o -o msgSend $(LDFLAGS)

server:	server.o
	$(CC) $(CFLAGS) server.o  -o server -levent

spRec:	spRec.o cJSON.o
	$(CC) $(CFLAGS) spRec.o cJSON.o  -o spRec -lm

clean:
	rm -f *.o main main_plugin msgSend server spRec


dep:
	$(CC) -M $(SOURCES) $(CFLAGS)  > .depend

-include .depend
