#include "../include/crpto.hpp"
CryptoHandler::CryptoHandler(const std::string& k) : key(k) {}

void CryptoHandler::setKey(const std::string& newKey){
    key=newKey;
    return;
}
std::string CryptoHandler::encrypt(const std::string& data){
    std::string result = data;
    for (size_t i = 0; i < data.size(); ++i)
        result[i] = data[i] ^ key[i % key.size()];
    return result;
}
std::string CryptoHandler::decrypt(const std::string& data){
    return encrypt(data);
}