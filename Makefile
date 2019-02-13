CC = gcc

.PHONY: clean

all: ThGME

main.o: main.c
	$(CC) -c $< -o $@	
ThGME: main.o
	$(CC) $< -o $@  
clean: 
	rm -f *.o ThGME


