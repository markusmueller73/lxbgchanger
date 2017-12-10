CC=gcc
CFLAGS=-I.
SRC=bgchanger.c
DEPS=bgchanger.h
OBJ=bgchanger.o
EXE=lxbgc

bgchanger: $(SRC) $(DEPS) 
	$(CC) -Wall -O3 -o $(EXE) $(SRC) $(CFLAGS)
	
debug: $(SRC) $(DEPS)
	$(CC) -Wall -DDEBUG -o $(EXE) $(SRC) $(CFLAGS)

