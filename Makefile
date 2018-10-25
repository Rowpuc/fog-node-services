CC = gcc
CFLAGS = -Wall -I/usr/local/include/libmongoc-1.0 -I/usr/local/include/libbson-1.0 -L/usr/local/lib 
LDFLAGS = -lmosquitto 
LDLIBS = -lmongoc-1.0 -lbson-1.0


#mongoserver: server.o
#	$(CC) -o server server.o

mosq-client: mosq-client.o
	$(CC) $(CFLAGS) $(LDLIBS) -o mosq-client mosq-client.c $(LDFLAGS)
.PHONY: clean

clean:
	rm -f *o server

