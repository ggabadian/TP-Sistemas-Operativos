all: esi 

esi: esi.o
		gcc -o esi ESI.o
esi.o: ESI.c
		gcc -c ESI.c

cleans:
		@rm -f esi.o