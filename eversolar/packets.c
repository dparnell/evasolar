//
//  packets.c
//  eversolar
//
//  Created by Daniel Parnell on 29/09/13.
//  Copyright (c) 2013 Daniel Parnell. All rights reserved.
//

#include <stdio.h>
#include <memory.h>
#include <err.h>
#include "eversolar.h"

int build_packet(unsigned char source_address, unsigned char dest_address, unsigned char control_code, unsigned char function_code, unsigned char* data, unsigned char data_length, unsigned char* buffer) {
    int i;
    unsigned int checksum;
    
    // the packet header
    buffer[0] = 0xAA;
    buffer[1] = 0x55;
    // source address
    buffer[2] = source_address;
    buffer[3] = 0;
    // dest address
    buffer[4] = 0;
    buffer[5] = dest_address;
    buffer[6] = control_code;
    buffer[7] = function_code;
    buffer[8] = data_length;
    // copy the data
    if(data_length>0) {
        memcpy(&buffer[9], data, data_length);
    }
    
    checksum = 0;
    for(i=0; i<9+data_length; i++) {
        checksum += buffer[i];
    }
    
    buffer[9+data_length] = checksum >> 8;
    buffer[10+data_length] = checksum & 0xff;
    
    return data_length + 11;
}

char packet_is_valid(unsigned char *buffer, int size) {
    unsigned short checksum = 0;
    
    for(int i=0; i<size-2; i++) {
        checksum += buffer[i];
    }
    
    return checksum == ((buffer[size-2] << 8) | buffer[size-1]);
}

void dump_hex(const unsigned char* buffer, int size) {
    for(int i=0; i<size; i++) {
        printf("%02x ", buffer[i]);
    }
    
    printf("\n");
}

int send_request(Connector* connector, void* state, const unsigned char* buffer, int send_size, unsigned char* result, int result_size, int count) {
    for(int i=0; i<count; i++) {
        printf("Sending: ");
        dump_hex(buffer, send_size);
        
        int got_size = connector->send_request(state, buffer, send_size, result, result_size);
        if(got_size > 0) {
            printf("GOT: ");
            dump_hex(result, got_size);
            
            if(packet_is_valid(result, got_size)) {
                return got_size;
            }
            
            printf("packet is invalid\n");
        } else {
            warn("Read failed");
            
        }
    }
    
    return 0;
}
