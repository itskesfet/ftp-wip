#pragma once

#include <string>
#include <ctime>

enum class Privilege {
    GUEST,
    USER,
    ADMIN
};

class Hut{
protected:
    std::string hutid;
    std::string passwordHash;
    bool logedIn;
    Privilege privilage;
    std::time_t last_login;
public:

    Hut(const std::string& uname, const std::string& passHash);
    ~Hut();
    
    static std::string hashf(const std::string pass);
    bool login(const std::string& password);
    void logout();
    bool isLoggedIn() const;
    std::string getUsername() const;
    void updateActivity();
    Privilege getPower() const;
    bool upgradePower(Privilege newPriv);
};