CC = gcc
CFLAGS = -Wall -I. -I/usr/local/include/libmongoc-1.0 -I/usr/local/include/libbson-1.0
LDFLAGS = -lmosquitto -lmongoc-1.0 -lbson-1.0


#mongoserver: server.o
#	$(CC) -o server server.o

parser: parser.o
	$(CC) -o parser parser.o $(LDFLAGS)
.PHONY: clean

clean:
	rm -f *o server

