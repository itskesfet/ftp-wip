#pragma once

#include <string>

class Logger {
private:
    static std::string logFile; 

public:
    static void log(const std::string& message, const std::string& level = "INFO");
};

/*
use - Logger::log("Server started", "INFO");
*/