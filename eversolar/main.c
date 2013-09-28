//
//  main.c
//  eversolar
//
//  Created by Daniel Parnell on 28/09/13.
//  Copyright (c) 2013 Daniel Parnell. All rights reserved.
//

#include <stdio.h>
#include <memory.h>
#include <err.h>
#include "eversolar.h"

extern Connector eth2ser_connector;

Connector* connectors[] = {
    &eth2ser_connector,
    NULL
};

int build_packet(unsigned char source_address, unsigned char dest_address, unsigned char control_code, unsigned char function_code, unsigned char* data, unsigned char data_length, unsigned char* buffer) {
    int i;
    unsigned short checksum;
    
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
    for(i=0; i<8+data_length; i++) {
        checksum += buffer[i];
    }
    
    buffer[9+data_length] = checksum >> 8;
    buffer[10+data_length] = checksum & 0xff;
    
    return data_length + 11;
}


void dump_hex(const unsigned char* buffer, int size) {
    for(int i=0; i<size; i++) {
        printf("%02x ", buffer[i]);
    }
    
    printf("\n");
}

char send_request(Connector* connector, void* state, const unsigned char* buffer, int send_size, unsigned char* result, int result_size, int count) {
    for(int i=0; i<count; i++) {
        printf("Sending: ");
        dump_hex(buffer, send_size);

        int got_size = connector->send_request(state, buffer, send_size, result, result_size);
        if(got_size > 0) {
            printf("GOT: ");
            dump_hex(result, got_size);
            
            return got_size;
        } else {
            warn("Read failed");
            
        }
    }
    
    return 0;
}

int main(int argc, const char * argv[])
{
    unsigned char buffer[1024];
    unsigned char result_buffer[256];
    
    if(argc > 1) {
        int i = 0;
        Connector* connector = NULL;
        while(connectors[i] && connector == NULL) {
            if(connectors[i]->can_handle(argv[1])) {
                connector = connectors[i];
                break;
            }
            
            i++;
        }
        
        if(connector) {
            printf("Connecting...\n");
            void* connector_state = connector->connect(argv[1]);
            
            if(connector_state) {
                printf("Connected\n");
                int send_size = build_packet(0x01, 0x00, 0x10, 0x00, NULL, 0, buffer);
                // int send_size = build_packet(0x01, 0x10, 0x11, 0x02, NULL, 0, buffer);
                
                send_request(connector, connector_state, buffer, send_size, result_buffer, sizeof(result_buffer), 10);
                connector->disconnect(connector_state);
            } else {
                printf("unable to connect to '%s'\n", argv[1]);
            }
        } else {
            printf("no connector can handle '%s'\n", argv[1]);
        }
    } else {
        printf("invalid args\n");
    }
    return 0;
}

