#include "../include/logfoo.hpp"

#include <fstream>
#include <chrono>
#include <ctime>
#include <ostream>

std::string Logger::logFile = "../debug/logfile.txt";
void Logger::log(const std::string& message, const std::string& level){

    //open the file with append mode
    std::ofstream ofile(Logger::logFile,std::ios::app); 
    if (!ofile.is_open()) {
        return;
    }

    //Time stamp
    auto start = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(start);

    //Error passing formate
    ofile <<"["<<std::ctime(&now_time) <<"]    "<<"["<<level <<"]    "<< message << std::endl; 
}