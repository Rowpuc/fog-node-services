CC = gcc
CFLAGS = -Wall -I.
LDFLAGS = -lmosquitto


#mongoserver: server.o
#	$(CC) -o server server.o

parser: parser.o
	$(CC) -o parser parser.o $(LDFLAGS)
.PHONY: clean

clean:
	rm -f *o server

