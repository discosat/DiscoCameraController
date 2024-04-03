#include <iostream>
#include <vector>
#include <VmbCPP/VmbCPP.h>
#include <filesystem>
#include <ctime>
#include <bits/stdc++.h>
#include <chrono>

#include "common.hpp"
#include "message_queue.hpp"
#include "capture_controller.hpp"

extern "C" {
    #include "csp_server.h"
}

namespace fs = std::filesystem;

using namespace std::chrono;

std::string_view get_option(
    const std::vector<std::string_view>& args, 
    const std::string_view& option_name) {
    for (auto it = args.begin(), end = args.end(); it != end; ++it) {
        if (*it == option_name)
            if (it + 1 != end)
                return *(it + 1);
    }
    
    return "";
}

bool has_option(
    const std::vector<std::string_view>& args, 
    const std::string_view& option_name) {
    for (auto it = args.begin(), end = args.end(); it != end; ++it) {
        if (*it == option_name)
            return true;
    }
    
    return false;
}

std::vector<std::string> split_string(std::string input){
    std::istringstream iss(input);
    std::string word;
    std::vector<std::string> words;

    while (std::getline(iss, word, ',')) {
        words.push_back(word);
    }

    return words;
}

void print_usage(){
    std::string help = R"""(Usage: Disco2CameraControl -i INTERFACE -p DEVICE -a NODE

Description:
  Listen to a capture message with the following options:

Arguments:
  -i INTERFACE   The connection interface to utilize, can be one of: zmq, can, kiss.
                 Default: zmq
  -d DEVICE      Device connection interface.
                 Default: localhost
  -p PORT        Port that the server listens to for command packets.
                 Default: 10
  -n NODE        CSP node address.
                 Default: 2
)""";

    std::cout << help << std::endl;
}

int main(int argc, char *argv[], char *envp[]){
    // parse arguments
    const std::vector<std::string_view> args(argv, argv + argc);
    
    if(has_option(args, "-h")){
        print_usage();
        return 0;
    }
    
    const std::string_view interface_arg = get_option(args, "-i");
    const std::string_view device_arg = get_option(args, "-d");
    const std::string_view node_arg = get_option(args, "-n");
    const std::string_view port_arg = get_option(args, "-p");
    int node = 2, port = 10;
    std::string device = "localhost", interface = "zmq";

    if(node_arg.size() > 0){
        node = std::atoi(std::string(node_arg).c_str());
    }

    if(port_arg.size() > 0){
        port = std::atoi(std::string(port_arg).c_str());
    }

    if(device_arg.size() > 0){
        device = std::string(device_arg);
    }

    if(interface_arg.size() > 0){
        interface = std::string(interface_arg);
    }

    CSPInterface interfaceConfig;
    interfaceConfig.Interface = StringToCSPInterface(interface.c_str());
    interfaceConfig.Device = device.c_str();
    interfaceConfig.Node = node;
    interfaceConfig.Port = port;

    CaptureController* captureController = new CaptureController();
    server_start(&interfaceConfig, captureController->CaptureCallback, (void*)captureController);

    delete captureController;
    return 0;
}