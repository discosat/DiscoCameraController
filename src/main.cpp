#include <VmbCPP/VmbCPP.h>
#include <bits/stdc++.h>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <vector>

#include "capture_controller.hpp"
#include "common.hpp"
#include "message_queue.hpp"

extern "C" {
#include "csp_server.h"
#include "temperature_bridge.h"
#include <vmem/vmem_file.h>
}

extern "C" vmem_t vmem_config;

namespace fs = std::filesystem;

using namespace std::chrono;

std::string_view get_option(const std::vector<std::string_view> &args,
                            const std::string_view &option_name) {
  for (auto it = args.begin(), end = args.end(); it != end; ++it) {
    if (*it == option_name)
      if (it + 1 != end)
        return *(it + 1);
  }

  return "";
}

bool has_option(const std::vector<std::string_view> &args,
                const std::string_view &option_name) {
  for (auto it = args.begin(), end = args.end(); it != end; ++it) {
    if (*it == option_name)
      return true;
  }

  return false;
}

std::vector<std::string> split_string(std::string input) {
  std::istringstream iss(input);
  std::string word;
  std::vector<std::string> words;

  while (std::getline(iss, word, ',')) {
    words.push_back(word);
  }

  return words;
}

void print_usage() {
  std::string help =
      R"""(Usage: Disco2CameraControl -i INTERFACE -p DEVICE -a NODE

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
Optional:
  -D             Debug option used during experiments
  -M MESSAGE     Debug message to simulate a CSP param callback
  -v DIR         Directory path for vmem files.
                 Default: current directory
)""";

  std::cout << help << std::endl;
}

int main(int argc, char *argv[], char *envp[]) {
  // parse arguments
  const std::vector<std::string_view> args(argv, argv + argc);

  if (has_option(args, "-h")) {
    print_usage();
    return 0;
  }

  bool debug = has_option(args, "-D");
  // const std::string_view debug_message_arg = get_option(args, "-M");

  const std::string_view interface_arg = get_option(args, "-i");
  const std::string_view device_arg = get_option(args, "-d");
  const std::string_view node_arg = get_option(args, "-n");
  const std::string_view port_arg = get_option(args, "-p");
  const std::string_view vmem_dir_arg = get_option(args, "-v");
  int node = 2, port = 10;
  std::string device = "localhost", interface = "zmq";

  if (node_arg.size() > 0) {
    node = std::atoi(std::string(node_arg).c_str());
  }

  if (port_arg.size() > 0) {
    port = std::atoi(std::string(port_arg).c_str());
  }

  if (device_arg.size() > 0) {
    device = std::string(device_arg);
  }

  if (interface_arg.size() > 0) {
    interface = std::string(interface_arg);
  }

  // Set vmem directory path if provided
  if (vmem_dir_arg.size() > 0) {
    std::string vmem_dir = std::string(vmem_dir_arg);
    std::string vmem_full_path = vmem_dir + "/config.vmem";
    ((vmem_file_driver_t *)vmem_config.driver)->filename = strdup(vmem_full_path.c_str());
    std::cout << "Using vmem directory: " << vmem_dir << std::endl;
    std::cout << "Config vmem file: " << vmem_full_path << std::endl;
  }

  CaptureController *captureController = new CaptureController();

  // Initialize temperature controller
  init_temperature_controller();

  if (!debug) {
    CSPInterface interfaceConfig;
    interfaceConfig.Interface = StringToCSPInterface(interface.c_str());
    interfaceConfig.Device = device.c_str();
    interfaceConfig.Node = node;
    interfaceConfig.Port = port;

    server_start(&interfaceConfig, captureController->CaptureCallback,
                 (void *)captureController);
  } // else {
  //     std::cout << "Testing camera controller..." << std::endl;
  //     std::string debug_message = std::string(debug_message_arg);
  //     u_int16_t error = 0;
  //     captureController->CaptureCallback(debug_message.data(),
  //     captureController, &error);
  // }

  // Cleanup temperature controller
  cleanup_temperature_controller();
  delete captureController;
  return 0;
}
