#pragma once

#include <string>

class CryptoHandler{
protected:
    std::string key;
public:
    CryptoHandler(const std::string& key = "defaultkey"); 
    void setKey(const std::string& newKey);
    std::string encrypt(const std::string& data);
    std::string decrypt(const std::string& data);
};