//
//  commands.c
//  eversolar
//
//  Created by Daniel Parnell on 29/09/13.
//  Copyright (c) 2013 Daniel Parnell. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "eversolar.h"

static int execute_scan(Connector* connector, void* state, const char** params) {
    unsigned char buffer[1024];
    unsigned char result_buffer[256];
    char name_buffer[MAX_INVERTER_NAME_LENGTH + 1];

    for(int i=0; i<10; i++) {
        // build up an OFFLINE QUERY packet
        int send_size = build_packet(0x01, 0x00, REGISTER, OFFLINE_QUERY, NULL, 0, buffer);
        
        int result_size = send_request(connector, state, buffer, send_size, result_buffer, sizeof(result_buffer), 1);
        
        if(result_size) {
            if(result_buffer[7] == 0x80) {
                int index;
                int name_length = result_buffer[8];
                
                strncpy((char*)name_buffer, (char*)&result_buffer[9], name_length);
                name_buffer[name_length] = '\0';
                printf("Found inverter '%s'\n", name_buffer);
                
                index = add_inverter(name_buffer);
                if(index >= 0) {
                    name_buffer[name_length] = inverters[index].address;
                    
                    printf("Allocated address %02x\n", inverters[index].address);
                    // build up a REGISTER_ADDRESS packet
                    send_size = build_packet(0x01, 0x00, REGISTER, REGISTER_ADDRESS, (unsigned char*)name_buffer, name_length+1, buffer);
                    
                    result_size = send_request(connector, state, buffer, send_size, result_buffer, result_size, 1);
                }
            } else {
                printf("Unexcpected scan response\n");
            }
            
        }
    }
    return 0;
}

static Command scan_command = {
    "scan",
    execute_scan
};


static int unregister(Connector* connector, void* state, const char** params) {
    unsigned char buffer[1024];
    unsigned char result_buffer[256];
    unsigned int target_address;
    
    sscanf(*params, "%x", &target_address);
    int send_size = build_packet(0x01, target_address & 0xff, REGISTER, UNREGISTER_ADDRESS, NULL, 0, buffer);
    
    send_request(connector, state, buffer, send_size, result_buffer, sizeof(result_buffer), 10);
    
    return 1;
}

static Command unregister_command = {
    "unregister",
    unregister
};

static int reregister(Connector* connector, void* state, const char** params) {
    unsigned char buffer[1024];
    unsigned char result_buffer[256];
    
    int send_size = build_packet(0x01, 0x00, REGISTER, REREGISTER, NULL, 0, buffer);
    
    send_request(connector, state, buffer, send_size, result_buffer, sizeof(result_buffer), 10);
    
    return 0;
}

static Command reregister_command = {
    "reregister",
    reregister
};


static int list(Connector* connector, void* state, const char** params) {
    for(int i=0; i<inverter_count; i++) {
        printf("Inverter %d - %s - %02x\n", i, inverters[i].name, inverters[i].address);
    }
    return 0;
}

static Command list_command = {
    "list",
    list
};

typedef struct NormalInfo {
    int16_t inverter_temp;
    uint16_t e_today;
    uint16_t vpv1;
    uint16_t vpv2;
    uint16_t unknown_03;
    uint16_t ipv1;
    uint16_t ipv2;
    uint16_t e_total_high;
    uint16_t e_total_low;
    uint16_t pac;
    uint16_t h_total_high;
    uint16_t h_total_low;
    uint16_t mode;
    uint16_t unknown_0e;
    uint16_t unknown_0f;
    uint16_t unknown_10;
    uint16_t unknown_11;
    uint16_t unknown_12;
    uint16_t unknown_13;
    uint16_t unknown_14;
    uint16_t unknown_15;
    uint16_t unknown_16;
    uint16_t unknown_17;
    uint16_t unknown_18;
    uint16_t unknown_19;
    uint16_t sur_temp;
    uint16_t br_temp;
    uint16_t irr;
} NormalInfo;

static int query(Connector* connector, void* state, const char** params) {
    unsigned char buffer[1024];
    unsigned char result_buffer[256];

    for(int i=0; i<inverter_count; i++) {
        
        int send_size = build_packet(0x01, inverters[i].address, QUERY, READ_NORMAL_INFO, NULL, 0, buffer);
        if(send_request(connector, state, buffer, send_size, result_buffer, sizeof(result_buffer), 10) > 0) {
            unsigned char length = result_buffer[8];
            unsigned char* pos = result_buffer + 9;
            uint16_t values[256];
            
            for(int j=0; j<length; j += 2) {
                values[j>>1] = (pos[j] << 8) | pos[j+1];
            }

            NormalInfo* info = (NormalInfo*)&values;
            printf("DATA: %s - Inverter Temp: %0.1f PV1: %0.1f PV2: %0.1f Ipv1: %0.1f Ipv2: %0.1f E-Today: %0.2f PAC: %d\n",
                   inverters[i].name,
                   info->inverter_temp * 0.1f,
                   info->vpv1 * 0.1f,
                   info->vpv2 * 0.1f,
                   info->ipv1 * 0.1f,
                   info->ipv2 * 0.1f,
                   info->e_today * 0.01f,
                   info->pac);
            
/*
            int value;
            char* name;
            char* units;
            int index = 0;
            float multiplier;
            
            printf("DATA: %s - ", inverters[i].name);
            
            while(length) {
                value = *pos << 8;
                pos++;
                value = value | *pos;
                pos++;
                
                switch (index) {
                    case 0x00:
                        name = "Temperaure";
                        units = "C";
                        multiplier = 0.1f;
                        break;
                    case 0x01:
                        name = "Vpv1";
                        units = "V";
                        break;
                    case 0x02:
                        name = "Vpv2";
                        units = "V";
                        break;
                    case 0x04:
                        name = "Ipv1";
                        units = "A";
                        multiplier = 0.1;
                        break;
                    case 0x05:
                        name = "Ipv2";
                        units = "A";
                        multiplier = 0.1;
                        break;
                    case 0x07:
                        name = "E-Total H";
                        units = "";
                        multiplier = 1;
                        break;
                    case 0x08:
                        name = "E-Total L";
                        units = "";
                        multiplier = 1;
                        break;
                    case 0x09:
                        name = "h-Total H";
                        units = "";
                        multiplier = 1;
                        break;
                    case 0x0a:
                        name = "l-Total L";
                        units = "";
                        multiplier = 1;
                        break;
                    case 0x0b:
                        name = "Pac";
                        units = "W";
                        multiplier = 1;
                        break;
                    case 0x0c:
                        name = "Mode";
                        units = "";
                        multiplier = 1;
                        break;
                    case 0x0d:
                        name = "E-Today";
                        units = "kW.Hr";
                        multiplier = 0.01;
                        break;
                        
                    default:
                        name = "Unknown";
                        units = "";
                        multiplier = 1.0f;
                        break;
                }
                
                length -= 2;
                index++;
                
                printf("%s: %f %s ", name, value * multiplier, units);
            }
            
            printf("\n");
 */
            
        }
    }
    return 0;
}

static Command query_command = {
    "query",
    query
};

static int data(Connector* connector, void* state, const char** params) {
    unsigned char buffer[1024];
    unsigned char result_buffer[256];
    
    for(int i=0; i<inverter_count; i++) {
        
        int send_size = build_packet(0x01, inverters[i].address, QUERY, READ_FIX_SIZE, NULL, 0, buffer);
        if(send_request(connector, state, buffer, send_size, result_buffer, sizeof(result_buffer), 10) > 0) {
        }
    }
    
    return 0;
}

static Command data_command = {
    "data",
    data
};

Command* commands[] = {
    &scan_command,
    &unregister_command,
    &reregister_command,
    &list_command,
    &query_command,
    &data_command,
    NULL
};
