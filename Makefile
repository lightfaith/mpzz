CC=scan-build g++
#CC=g++
CFLAGS= -g -Wall

all: spf gen

spf: spf.cpp classes.o lib.o parser.o
	$(CC) $(CFLAGS) -o $@ $^

classes.o: classes.cpp lib.o
	$(CC) $(CFLAGS) -c $^

lib.o: lib.cpp
	$(CC) $(CFLAGS) -c $^

parser.o: parser.cpp lib.o classes.o
	$(CC) $(FLAGS) -c $^

gen: gen.cpp
	$(CC) $(CFLAGS) -o $@ $^

clean: 
	rm -f spf gen *.o
