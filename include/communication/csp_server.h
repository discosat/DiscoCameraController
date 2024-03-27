#ifndef CSP_SERVER_H
#define CSP_SERVER_H

#include <stdio.h>  
#include <string.h>  
#include <pthread.h>
#include <param/param.h>
#include "param_config.h"

// callback for image capture
typedef void(*CallbackFunc)(char*, void*, uint16_t*);

typedef enum CSPInterfaceType {
    ZMQ = 0,
    CAN = 1,
    KISS = 2
} CSPInterfaceType;

typedef struct {
    CSPInterfaceType Interface;
    const char *Device;
    int Node;
    int Port;
} CSPInterface;

/// @brief Creates a CSPInterfaceType, which is an enum of the interface to connect over
/// @param name 
/// @return CSPInterfaceType: type of interface to connect to
inline CSPInterfaceType StringToCSPInterface(const char* name)
{
    if(strcmp(name, "zmq") == 0){
        return ZMQ;
    } else if(strcmp(name, "can") == 0){
        return CAN;
    } else if(strcmp(name, "kiss") == 0){
        return KISS;
    } else {
        return ZMQ;
    }
}

void capture_param_callback(struct param_s *param, int offset);

void server_start(CSPInterface *interfaceConfig, CallbackFunc callback, void* obj);

#endif
