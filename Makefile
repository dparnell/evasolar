all:
	mkdir -p bin
	g++ -fpermissive eversolar/main.c eversolar/connectors.c eversolar/commands.c eversolar/packets.c eversolar/inverters.c -o bin/eversolar
