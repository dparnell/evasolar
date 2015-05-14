# evasolar
Code to talk to an eversolar inverter via an RS485 network

The code should compile using `make` on a Linux machine with GCC installed.

It supports communicating with an eversolar inverter attached to a serial to ethernet/wifi adapter, however other transports can be added easily.

## commands

### register

Search the RS485 network for any attached devices
Any devices found will be saved to `/tmp/eversolar.state`

### scan

Scan the RS485 for any registered devices

### query

Query the data for any registered devices

## usage

The way it is typically used is to combine all three commands as follows

    eversolar eth2ser://$SOLAR_HOST:$SOLAR_PORT reregister scan query

This will spit out the raw data sent and received along and print the results of any valid packets.
The basic idea is to then pump the output of the command through other Unix tools like *sed*, *awk* and *grep* to grab out the data you need.
