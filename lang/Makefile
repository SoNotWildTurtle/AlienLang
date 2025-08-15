CC?=gcc
CFLAGS=-O2 -Wall

all: alienc

alienc: compiler.o main.o
	$(CC) $(CFLAGS) $^ -o $@

compiler.o: compiler.c compiler.h ../os/kernel/alienvm.h
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c compiler.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o alienc
