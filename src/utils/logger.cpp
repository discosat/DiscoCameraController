#include "logger.hpp"
#include <string>

// C-compatible logging functions implementation
extern "C" {

void log_info(const char* msg) {
    DiscoLogger::info(std::string(msg));
}

void log_success(const char* msg) {
    DiscoLogger::success(std::string(msg));
}

void log_warning(const char* msg) {
    DiscoLogger::warning(std::string(msg));
}

void log_error(const char* msg) {
    DiscoLogger::error(std::string(msg));
}

void log_debug(const char* msg) {
    DiscoLogger::debug(std::string(msg));
}

void log_camera(const char* msg) {
    DiscoLogger::camera(std::string(msg));
}

void log_gpio(const char* msg) {
    DiscoLogger::gpio(std::string(msg));
}

} // extern "C"