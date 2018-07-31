COMPILER=gcc
BIN_PATH=bin
LOG_PATH=logs
LIBS = libs/protocolo.c -lcommons -pthread # Define referencias
CLIENTE = libs/socketClient.c
SERVIDOR = libs/socketServer.c

all: clean coordinador planificador instancia esi

#	mkdir -p $(BIN_PATH)
#	$(COMPILER) Coordinador/src/Coordinador.h Coordinador/src/Coordinador.c -o $(BIN_PATH)/Coordinador
#	$(COMPILER) Planificador/src/Planificador.h Planificador/src/Planificador.c -o $(BIN_PATH)/Planificador
#	$(COMPILER) Instancia/src/Instancia.h Instancia/src/Instancia.c -o $(BIN_PATH)/Instancia
#	$(COMPILER) ESI/src/ESI.h ESI/src/ESI.c -o $(BIN_PATH)/ESI

clean:
	rm -rf bin
	@rm -rf ${LOG_PATH}/*.log

coordinador:
	mkdir -p $(BIN_PATH)
	mkdir -p $(LOG_PATH)
	$(COMPILER) -g -Wall Coordinador/src/Coordinador.h Coordinador/src/Coordinador.c Coordinador/src/configCoordinador.c $(SERVIDOR) $(LIBS) -lm -o $(BIN_PATH)/Coordinador

planificador:
	mkdir -p $(BIN_PATH)
	mkdir -p $(LOG_PATH)
	$(COMPILER) -g -Wall Planificador/src/Planificador.h Planificador/src/Planificador.c Planificador/src/configPlanificador.c $(SERVIDOR) $(CLIENTE) $(LIBS) -o $(BIN_PATH)/Planificador

instancia:
	mkdir -p $(BIN_PATH)
	mkdir -p $(LOG_PATH)
	$(COMPILER) -g -Wall Instancia/src/Instancia.h Instancia/src/Instancia.c Instancia/src/configInstancia.c $(CLIENTE) $(LIBS) -lm -o $(BIN_PATH)/Instancia

esi:
	mkdir -p $(BIN_PATH)
	mkdir -p $(LOG_PATH)
	$(COMPILER) -g -Wall ESI/src/ESI.h ESI/src/ESI.c ESI/src/configESI.c $(CLIENTE) $(LIBS)-lparsi -o $(BIN_PATH)/ESI

installDependencies:
	cd ..; git clone https://github.com/sisoputnfrba/so-commons-library.git; cd so-commons-library; sudo make install
	cd ..; git clone https://github.com/sisoputnfrba/parsi.git; cd parsi; sudo make install