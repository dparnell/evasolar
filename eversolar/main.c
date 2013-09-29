//
//  main.c
//  eversolar
//
//  Created by Daniel Parnell on 28/09/13.
//  Copyright (c) 2013 Daniel Parnell. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include "eversolar.h"

#define STATE_FILE_NAME "/tmp/eversolar.state"

int main(int argc, const char * argv[])
{
    /*
    unsigned char data[] = { 0x42, 0x38, 0x38, 0x35, 0x34, 0x30, 0x30, 0x41, 0x31, 0x32, 0x39, 0x52, 0x30, 0x31, 0x31, 0x36, 0x10 };
    unsigned char buffer[1024];
    
    int size = build_packet(0x01, 0, REGISTER, REGISTER_ADDRESS, data, sizeof(data), buffer);
    dump_hex(buffer, size);
    if(packet_is_valid(buffer, size)) {
        printf("OK\n");
    } else {
        printf("BOOM\n");
    }
    
    return 0;
 */
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
            load_inverters(STATE_FILE_NAME);
            printf("Connecting...\n");
            void* connector_state = connector->connect(argv[1]);
            
            if(connector_state) {
                printf("Connected\n");
                
                argv += 2;
                argc -= 2;
                while(argc && argv) {
                    Command* cmd = NULL;
                    i = 0;
                    
                    while(commands[i] && cmd == NULL) {
                        if(strcmp(commands[i]->name, *argv) == 0) {
                            cmd = commands[i];
                        }
                        
                        i++;
                    }
                    
                    if(cmd) {
                        int result;
                        argv++;
                        argc--;
                        
                        result = cmd->execute(connector, connector_state, argv);
                        
                        if(result < 0) {
                            printf("Command '%s' failed\n", *argv);
                            return -1;
                        } else {
                            argc -= result;
                            argv += result;
                        }
                    
                    } else {
                        printf("Unknown command: '%s'\n", *argv);
                        break;
                    }
                }
                connector->disconnect(connector_state);
                
                save_inverters(STATE_FILE_NAME);
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

