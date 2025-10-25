#include "../include/files.hpp"
#include "../include/logfoo.hpp"


#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstdio>
#include <string>

FileManager::FileManager(const std::string& path) : storagePath(path){}

bool FileManager::saveFile(const std::string& filename, const std::string& data, Privilege owner){
    if (std::filesystem::exists(filename)) {
        Logger::log("File already exists: " + filename, "ERROR");
        return false;
    }
    std::ofstream fstr(filename, std::ios::binary);
    
    std::string encData = crypto.encrypt(data);
    fstr.write(encData.c_str(), encData.size());
    fstr.close();

    Logger::log("File saved and encrypted: " + filename, "INFO");
    return true;
}
bool FileManager::readFile(const std::string& filename, std::string& outData){
    std::ifstream fstr;
    fstr.open(filename,std::ios::in);
    if (!fstr.is_open()) {
        Logger::log("File not found: " + filename, "ERROR");
        return false;
    }
    outData.assign((std::istreambuf_iterator<char>(fstr)),std::istreambuf_iterator<char>());
    fstr.close();
    Logger::log("File read successfully: " +filename,"INFO");
    return true;
};
bool FileManager::deleteFile(const std::string& filename){
    const char* file = filename.c_str();
    if(std::remove(file)==0){
        Logger::log("Removed: " +filename, "INFO");
        return true;
    }else {
        Logger::log("Failed to remove file: " + filename, "ERROR");
        return false;
    }
};
std::string FileManager::listFiles(std::string& dir){
    std::string cwd = dir;
    std::string listofFile("");
    for(auto& items : std::filesystem::directory_iterator(cwd)){
        if (items.is_directory()) listofFile.append("[DIR] ");
        else if (items.is_regular_file()) listofFile.append("[FILE] ");
        listofFile.append(items.path().filename().string());
        listofFile.append("\n");     
    }
    return listofFile;
};

void FileManager::setCryptoKey(const std::string& key){
    crypto.setKey(key);
};
bool FileManager::fileExists(const std::string& filename) {
    return std::filesystem::exists(filename);
};