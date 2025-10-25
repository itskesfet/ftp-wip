#pragma once
#include <string>
//#include <unordered_map>

#include "../include/crpto.hpp"
#include "../include/hut.hpp"

class FileManager {
private:
    std::string storagePath;
    CryptoHandler crypto;
    //std::unordered_map<std::string, Privilege> filePrivileges; 

public:
    FileManager(const std::string& path = "../storage");

    bool saveFile(const std::string& filename, const std::string& data, Privilege owner);
    bool readFile(const std::string& filename, std::string& outData);
    bool deleteFile(const std::string& filename);
    std::string listFiles(std::string& dir);
    void setCryptoKey(const std::string& key);
    bool fileExists(const std::string& filename);
};
