#include <iostream>
#include <string>
#include <sstream>
#include <vector>

typedef struct CaptureMessage {
    int NumberOfImages;
    int Exposure;
    int ISO;
    std::string Camera;
} CaptureMessage;

void parseVariables(const std::string& input, CaptureMessage& message) {
    std::vector<std::string> pairs;
    std::stringstream ss(input);
    std::string pair;
    while (std::getline(ss, pair, ';')) {
        pairs.push_back(pair);
    }

    for (const auto& p : pairs) {
        std::istringstream iss(p);
        std::string variable, value;
        std::getline(iss, variable, '=');
        std::getline(iss, value);
        
        variable.erase(0, variable.find_first_not_of(" \t\n\r\f\v"));
        variable.erase(variable.find_last_not_of(" \t\n\r\f\v") + 1);
        value.erase(0, value.find_first_not_of(" \t\n\r\f\v"));
        value.erase(value.find_last_not_of(" \t\n\r\f\v") + 1);
        
        // Check variable name and assign value -> kinda dirty...
        if (variable == "NUM_IMAGES") {
            message.NumberOfImages = std::stoi(value);
        } else if (variable == "ISO") {
            message.ISO = std::stof(value);
        } else if (variable == "EXPOSURE") {
            message.Exposure = std::stoi(value);
        } else if (variable == "CAMERA") {
            message.Camera = value;
        } else {
            continue;
        }
    }
}

int main() {
    std::string input = "CAMERA=1800 U-500c;NUM_IMAGES=20;EXPOSURE=55000;ISO=1.5;";
    std::string variable_1, variable_2;
    CaptureMessage message;
    
    // Parse the input string
    parseVariables(input, message);
    
    // Output the parsed variables
    std::cout << "Camera: " << message.Camera << std::endl;
    std::cout << "Exposure: " << message.Exposure << std::endl;
    std::cout << "ISO: " << message.ISO << std::endl;
    std::cout << "Number of images: " << message.NumberOfImages << std::endl;
    
    return 0;
}
