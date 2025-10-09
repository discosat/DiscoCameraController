#ifndef LOGGER_HPP
#define LOGGER_HPP

#ifdef __cplusplus
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <sstream>
#endif

// ANSI color codes for terminal output
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"
#define BOLD    "\033[1m"

#ifdef __cplusplus
// Log level definitions
enum class LogLevel {
    INFO,
    SUCCESS,
    WARNING,
    ERROR,
    DEBUG,
    CAMERA,
    GPIO
};

// Simple logger class for consistent formatting
class DiscoLogger {
public:
    static void log(LogLevel level, const std::string& message) {
        std::string prefix;
        std::string color;

        switch(level) {
            case LogLevel::INFO:
                prefix = "[INFO]    ";
                color = CYAN;
                break;
            case LogLevel::SUCCESS:
                prefix = "[SUCCESS] ";
                color = GREEN;
                break;
            case LogLevel::WARNING:
                prefix = "[WARNING] ";
                color = YELLOW;
                break;
            case LogLevel::ERROR:
                prefix = "[ERROR]   ";
                color = RED;
                break;
            case LogLevel::DEBUG:
                prefix = "[DEBUG]   ";
                color = MAGENTA;
                break;
            case LogLevel::CAMERA:
                prefix = "[CAMERA]  ";
                color = BLUE;
                break;
            case LogLevel::GPIO:
                prefix = "[GPIO]    ";
                color = WHITE;
                break;
        }

        // Get current time
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        // Format time
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
        ss << "." << std::setfill('0') << std::setw(3) << ms.count();

        // Output with color
        std::cout << color << BOLD << prefix << RESET
                  << "[" << ss.str() << "] "
                  << message << std::endl;
    }

    // Convenience functions
    static void info(const std::string& msg) { log(LogLevel::INFO, msg); }
    static void success(const std::string& msg) { log(LogLevel::SUCCESS, msg); }
    static void warning(const std::string& msg) { log(LogLevel::WARNING, msg); }
    static void error(const std::string& msg) { log(LogLevel::ERROR, msg); }
    static void debug(const std::string& msg) { log(LogLevel::DEBUG, msg); }
    static void camera(const std::string& msg) { log(LogLevel::CAMERA, msg); }
    static void gpio(const std::string& msg) { log(LogLevel::GPIO, msg); }
};
#endif // __cplusplus

// C-compatible logging functions for use in C files
#ifdef __cplusplus
extern "C" {
#endif

void log_info(const char* msg);
void log_success(const char* msg);
void log_warning(const char* msg);
void log_error(const char* msg);
void log_debug(const char* msg);
void log_camera(const char* msg);
void log_gpio(const char* msg);

#ifdef __cplusplus
}
#endif

#endif // LOGGER_HPP