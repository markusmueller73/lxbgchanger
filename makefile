CC=gcc
CFLAGS=-I.
DEPS=bgchanger.h
OBJ=bgchanger.o
EXE=lxbgc

%.o: %.c $(DEPS)
	$(CC) -Wall -O3 -c -o $@ $< $(CFLAGS)

bgchanger: $(OBJ) 
	$(CC) -Wall -O3 -o $(EXE) $^ $(CFLAGS)
	
