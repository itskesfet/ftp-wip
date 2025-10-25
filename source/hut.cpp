#include "../include/hut.hpp"




Hut::Hut(const std::string& uname, const std::string& passHash)
    : hutid(uname), passwordHash(passHash), logedIn(false), privilage(Privilege::GUEST), last_login(0){}

Hut::~Hut(){}

bool Hut::login(const std::string& password){
    if (logedIn) return false;
    if (hashf(password)==passwordHash) {
        logedIn= true;
        last_login = std::time(nullptr);
        return true;
    
    }
    return false;
}

void Hut::logout() {
    logedIn = false;
}

bool Hut::isLoggedIn() const{
    return logedIn;
}

Privilege Hut::getPower() const{
    return privilage;

}

std::string Hut::getUsername() const {
    return hutid;
}

std::string Hut::hashf(std::string pass){
    return pass;
}

bool Hut::upgradePower(Privilege newPriv){
    privilage = newPriv;
    return true;
    
}
