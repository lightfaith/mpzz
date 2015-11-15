CC=scan-build g++
#CC=g++
CFLAGS= -g -Wall 
ROOT=spf

$(ROOT): spf.cpp classes.o lib.o parser.o
	$(CC) $(CFLAGS) -o $@ $^

classes.o: classes.cpp lib.o
	$(CC) $(CFLAGS) -c $^

lib.o: lib.cpp
	$(CC) $(CFLAGS) -c $^

parser.o: parser.cpp lib.o classes.o
	$(CC) $(FLAGS) -c $^

clean: 
	rm -f $(ROOT) *.o
