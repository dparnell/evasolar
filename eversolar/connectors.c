//
//  eth2ser.c
//  eversolar
//
//  Created by Daniel Parnell on 28/09/13.
//  Copyright (c) 2013 Daniel Parnell. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <err.h>

#include "eversolar.h"

typedef struct Eth2SerState {
    int socket;
} Eth2SerState;

static char eth2ser_can_handle(const char* dest) {
    return strncmp(dest, "eth2ser://", 10) == 0;
}

static void* eth2ser_connect(const char* dest) {
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;
    Eth2SerState* state = NULL;
    
    memset(&hints, 0, sizeof hints);
    
    char* tmp = strdup(dest);
    char* host = tmp + 10;
    char* port = strstr(host, ":");
    if(port) {
        *port = '\0';
        port++;
    
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
        
        if ((status = getaddrinfo(host, port, &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        } else {
            int s = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
            
            if(connect(s, servinfo->ai_addr, servinfo->ai_addrlen) == 0) {
                struct timeval tv;
                tv.tv_sec = 1;
                tv.tv_usec = 0;
                setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));
                setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,sizeof(struct timeval));
                
                state = (Eth2SerState*)malloc(sizeof(Eth2SerState));
                state->socket = s;
            } else {
                warn("connection to '%s' on port '%s' failed", host, port);
            }
            freeaddrinfo(servinfo);
        }
    }
    
    return state;
}

static int eth2ser_send_request(void* state, const unsigned char* to_send, int send_size, unsigned char* result, int result_size) {
    int socket = ((Eth2SerState*)state)->socket;
    
    if(send(socket, to_send, send_size, 0) < 0) {
        warn("send failed");
        
        return -2;
    } else {
        // read all available bytes until we get a time out
        ssize_t count = 0;
        ssize_t bytes_read;
        while((bytes_read = recv(socket, result, result_size, 0)) >= 0) {
            result += bytes_read;
            result_size -= bytes_read;
            count += bytes_read;
        }
        
        if(count>0) {
            return (int)count;
        }
        return -1;
    }
}

static void eth2ser_disconnect(void* state) {
    shutdown(((Eth2SerState*)state)->socket, SHUT_RDWR);
}

static Connector eth2ser_connector = {
    eth2ser_can_handle,
    eth2ser_connect,
    eth2ser_send_request,
    eth2ser_disconnect
};


Connector* connectors[] = {
    &eth2ser_connector,
    NULL
};
