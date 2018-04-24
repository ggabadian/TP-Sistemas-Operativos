all: Instancia 

Instancia: Instancia.o
		gcc -o Instancia Instancia.o
Instancia.o: Instancia.c
		gcc -c Instancia.c

clean:
		@rm -f Instancia.o