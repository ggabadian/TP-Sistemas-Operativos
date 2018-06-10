COMPILER=gcc
BIN_PATH=bin
LIBS = -lcommons # Define referencias a commons

all: clean coordinador planificador instancia esi

#	mkdir -p $(BIN_PATH)
#	$(COMPILER) Coordinador/src/Coordinador.h Coordinador/src/Coordinador.c -o $(BIN_PATH)/Coordinador
#	$(COMPILER) Planificador/src/Planificador.h Planificador/src/Planificador.c -o $(BIN_PATH)/Planificador
#	$(COMPILER) Instancia/src/Instancia.h Instancia/src/Instancia.c -o $(BIN_PATH)/Instancia
#	$(COMPILER) ESI/src/ESI.h ESI/src/ESI.c -o $(BIN_PATH)/ESI

clean:
	rm -rf bin

coordinador:
	mkdir -p $(BIN_PATH)
	$(COMPILER) -g -Wall Coordinador/src/Coordinador.h Coordinador/src/Coordinador.c $(LIBS) -o $(BIN_PATH)/Coordinador

planificador:
	mkdir -p $(BIN_PATH)
	$(COMPILER) -g -Wall Planificador/src/Planificador.h Planificador/src/Planificador.c Planificador/src/configPlanificador.c $(LIBS) -o $(BIN_PATH)/Planificador

instancia:
	mkdir -p $(BIN_PATH)
	$(COMPILER) -g -Wall Instancia/src/Instancia.h Instancia/src/Instancia.c Instancia/src/configInstancia.c $(LIBS) -o $(BIN_PATH)/Instancia

esi:
	mkdir -p $(BIN_PATH)
<<<<<<< HEAD
	$(COMPILER) -g -Wall ESI/src/ESI.h ESI/src/ESI.c ESI/src/configESI.c $(LIBS)-lparsi -o $(BIN_PATH)/ESI
=======
	$(COMPILER) -g -Wall ESI/src/ESI.h ESI/src/ESI.c ESI/src/configESI.c $(LIBS)-lparsi -o $(BIN_PATH)/ESI
>>>>>>> 89fb337d40f756222d6e628e9e5071bf1bbe4721
