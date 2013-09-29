//
//  inverters.c
//  eversolar
//
//  Created by Daniel Parnell on 29/09/13.
//  Copyright (c) 2013 Daniel Parnell. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "eversolar.h"

char* state_file_name = NULL;
int inverter_count = 0;
Inverter inverters[MAX_INVERTERS];
static unsigned char next_inverter_address = 0x10;

int find_inverter(const char* name) {
    for(int i=0; i<inverter_count; i++) {
        if(strcmp(name, inverters[i].name) == 0) {
            return i;
        }
    }
    return -1;
}

void load_inverters(const char * filename) {
    FILE* infile = fopen(filename, "r");
    if(infile) {
        char buffer[1024];
        
        inverter_count = 0;
        next_inverter_address = 0x10;
        
        while(fgets(buffer, sizeof(buffer), infile)) {
            int address;
            char* pos = strstr(buffer, "=");
            if(pos) {
                *pos = '\0';
                sscanf(pos+1, "%x", &address);
                strncpy(inverters[inverter_count].name, buffer, MAX_INVERTER_NAME_LENGTH);
                inverters[inverter_count].address = address;
                inverter_count++;
                if(address >= next_inverter_address) {
                    next_inverter_address = address + 1;
                }
            }
        }
    }
}

void save_inverters(const char * filename) {
    FILE* outfile = fopen(filename, "w");
    if(outfile) {
        for(int i=0; i<inverter_count; i++) {
            fprintf(outfile, "%s=%02x\n", inverters[i].name, inverters[i].address);
        }
        fclose(outfile);
    }
}

int add_inverter(char * name) {
    int index = find_inverter(name);
    if(index < 0) {
        if(inverter_count < MAX_INVERTERS) {
            index =  inverter_count;
            inverter_count++;
            
            memset(&inverters[index], 0, sizeof(Inverter));
            strncpy(inverters[index].name, name, MAX_INVERTER_NAME_LENGTH);
            inverters[index].address = next_inverter_address;
            next_inverter_address++;
        }
    }
    return index;
}
