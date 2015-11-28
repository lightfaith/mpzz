CC=g++
CFLAGS= -Wall 
#CFLAGS= -fprofile-arcs -ftest-coverage
#CFLAGS= -g -Wall

all: spf gen


spf: spf.cpp classes.hpp classes.o lib.hpp lib.o parser.hpp parser.o
	$(CC) $(CFLAGS) -o $@ spf.cpp classes.o lib.o parser.o

classes.o: classes.cpp classes.hpp lib.o lib.hpp
	$(CC) $(CFLAGS) -c classes.cpp

lib.o: lib.cpp lib.hpp
	$(CC) $(CFLAGS) -c lib.cpp

parser.o: parser.cpp parser.hpp lib.o lib.hpp classes.o classes.hpp
	$(CC) $(CFLAGS) -c parser.cpp

gen: gen.cpp
	$(CC) $(CFLAGS) -o $@ $^

clean: 
	rm -f spf gen *.o
