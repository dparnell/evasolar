//
//  eversolar.h
//  eversolar
//
//  Created by Daniel Parnell on 28/09/13.
//  Copyright (c) 2013 Daniel Parnell. All rights reserved.
//

#ifndef eversolar_eversolar_h
#define eversolar_eversolar_h

typedef struct Connector {
    char (*can_handle)(const char* dest);
    void* (*connect)(const char* dest);
    int (*send_request)(void* state, const unsigned char* to_send, int send_size, unsigned char* result, int result_size);
    void (*disconnect)(void* state);
} Connector;

#endif
