#pragma once 
#include "../include/hut.hpp"
#include "../include/crpto.hpp"
#include "../include/files.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <string>
#include <functional>
#include <unordered_map>


struct Command {
    std::function<void(int, const std::vector<std::string>&)> action;
    Privilege requiredPrivilege;
    std::string description;
};

struct ClientInfo{
    int clientfd;
    Hut* hut;
    std::string metadata; 
};

class Host{
protected:
    uint16_t server_port;
    int server_socket;
    int epoll_fd;
    std::map<int, ClientInfo> client;
    std::unordered_map<std::string, Command> commandTable;
    std::unordered_map<std::string, Hut> huts;
    CryptoHandler crypto;
    FileManager filemanagement;
public:
    Host(int port);
    ~Host();
    bool setupServer();     // Init server_socket member , bind and setup non blocking state , Init epoll_fd
    bool runServer();
    bool acceptClient();
    bool listenOnPort();
    bool addFdToEpoll(int fd, uint32_t events);
    bool sendData(int clientSocket, const std::string& data);
    bool reciveData(int clientSocket, std::string& buffer);
    void initCommandTable();
    void handleCommand(int fd, const std::vector<std::string>& tokens);
};

std::string hashf(std::string pass);