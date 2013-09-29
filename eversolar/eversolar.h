//
//  eversolar.h
//  eversolar
//
//  Created by Daniel Parnell on 28/09/13.
//  Copyright (c) 2013 Daniel Parnell. All rights reserved.
//

#ifndef eversolar_eversolar_h
#define eversolar_eversolar_h

#define REGISTER 0x10

#define OFFLINE_QUERY 0x00
#define REGISTER_ADDRESS 0x01
#define UNREGISTER_ADDRESS 0x02
#define REREGISTER 0x04

#define QUERY 0x11

#define READ_NORMAL_INFO 0x02
#define READ_FIX_SIZE 0x10

typedef struct Connector {
    char (*can_handle)(const char* dest);
    void* (*connect)(const char* dest);
    int (*send_request)(void* state, const unsigned char* to_send, int send_size, unsigned char* result, int result_size);
    void (*disconnect)(void* state);
} Connector;

extern Connector* connectors[];

int build_packet(unsigned char source_address, unsigned char dest_address, unsigned char control_code, unsigned char function_code, unsigned char* data, unsigned char data_length, unsigned char* buffer);
char packet_is_valid(unsigned char *buffer, int size);
void dump_hex(const unsigned char* buffer, int size);
int send_request(Connector* connector, void* state, const unsigned char* buffer, int send_size, unsigned char* result, int result_size, int count);


typedef struct Command {
    char* name;
    int (*execute)(Connector* connector, void* state, const char** params);
} Command;

extern Command* commands[];

#define MAX_INVERTER_NAME_LENGTH 64

typedef struct Inverter {
    char name[MAX_INVERTER_NAME_LENGTH];
    unsigned char address;
} Inverter;

#define MAX_INVERTERS 100

extern int inverter_count;
extern Inverter inverters[MAX_INVERTERS];

int find_inverter(const char* name);
void load_inverters(const char * filename);
void save_inverters(const char * filename);
int add_inverter(char * name);

#endif
