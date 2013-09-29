all:
	mkdir -p bin
	gcc eversolar/main.c eversolar/connectors.c eversolar/commands.c eversolar/packets.c eversolar/inverters.c -o bin/eversolar
