#include "../include/host.hpp"
#include "../include/logfoo.hpp"
#include "../include/hut.hpp"
#include "../include/lexer.hpp"


#include <cstddef>
#include <cstring>
//#include <fstream>
#include <iostream>
#include <pthread.h>
#include <string>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <vector>

Host::Host(int port) : server_port(port), server_socket(-1) {
    std::cout << "Host initialized on port " << server_port << std::endl;
}

Host::~Host() {
    if (server_socket != -1) {
        close(server_socket);
        std::cout << "Server socket closed." << std::endl;
    }
}

void Host::initCommandTable(){
    
    
    std::cout << "server IP: "<<server_port<<"\n";


    commandTable["PING"] = {
        [this](int fd, const std::vector<std::string>& tokens) {
            sendData(fd, "PING REPLY\n");
        },
        Privilege::GUEST,
        "Respond ping"
    };
    commandTable["ECHO"] = {
        [this](int fd,const std::vector<std::string>& tokens) {
            sendData(fd, "ECHO");
        },
        Privilege::GUEST,
        "Echo"
    };
    commandTable["HELP"] = {
        [this](int fd,const std::vector<std::string>& tokens) {
            sendData(fd, "HELP RES");
        },
        Privilege::GUEST,
        "def later"
    };
    commandTable["REGISTER"]={
        [this](int fd, const std::vector<std::string>& token){
            if (token.size()<3) {
                std::cout << "Insufficent elements in command. REGISTER <UserName> <Password>";
            }
            const std::string uname = token[1];
            const std::string password = Hut::hashf(token[2]);

            auto it = huts.find(uname);
            if(it==huts.end()){
                huts.emplace(uname , Hut(uname , password)  );
                Logger::log("Added User: " + uname , "INFO");

            }else {
                //no action user allredy exists
            }
        },
        Privilege::GUEST,
        "Reg guest."
    };
    commandTable["LOGIN"]={
        [this](int fd,const std::vector<std::string>& token){
            if (token.size()<2) {
                std::cout << "Insufficent elements in command. LOGIN <UserName> <Password>";
            }
            const std::string uname = token[1];
            const std::string password = token[2];

            auto it=huts.find(uname);
            if (it!=huts.end()&&(it->second.login(password))) {
                Logger::log("User: " + uname + "Logind into the Server. " , "INFO");
            }else {
                Logger::log("User: " + uname + "Invalid login credentials. " , "ERROR");
            }
        },
        Privilege::USER,
        "Login User."
    };
    commandTable["LOGOUT"]={
        [this](int fd,const std::vector<std::string>& token){
            auto it = client.find(fd);
            if (it == client.end()) {
                Logger::log("Client not found. " , "ERROR");
                return;
            }
            ClientInfo& info = it->second;
            if(!info.hut|| !info.hut->isLoggedIn()){
                Logger::log("You are not logged in. " , "ERROR");

            }
        info.hut->logout();
        info.hut = nullptr; 
        Logger::log("You have been logged out successfully. " , "INFO");
        },
        Privilege::USER,
        "Login User."
    };
    commandTable["BROADCAST"]={
        [this](int fd,const std::vector<std::string>& token){
        if (token.size() < 2) {
            Logger::log("Insufficient elements in command. BROADCAST <message>", "ERROR");
            return;
        }
        Logger::log("Insufficent elements in The command. BROADCAST <message> " , "ERROR");
            std::string message;
            for (size_t i = 1; i < token.size(); ++i) {
                if (i > 1) message += " ";
                message += token[i];
            }
            for (auto& e : client) {
                ClientInfo info = e.second;
                if(sendData(info.clientfd, token[1])==true){
                    Logger::log("Sent bytes to Client "+std::to_string(info.clientfd), "INFO");
                }
            
            }
        },
        Privilege::ADMIN,
        "Brodcast Message."
    };
    commandTable["PROMOTE"]={
        [this](int fd,const std::vector<std::string>& token){
            if (token.size()<1) {
            
            }
            auto uname = token[1];
            auto power = token[2];
            auto it = client.find(fd);
            if (it == client.end()) {
                Logger::log("Unable to Find User. " , "ERROR");
                return;
            }
            ClientInfo& info = it->second;
            Hut* hut = info.hut;
            if (!hut || !hut->isLoggedIn()) {
                Logger::log("You must be logged in to promote others.", "ERROR");
                return;
            }
            auto target = huts.find(uname);
            if (target == huts.end()) {
                Logger::log("Target user not found.", "ERROR");
                return;
            }
            
            Privilege newPower;
            if (power == "ADMIN") newPower = Privilege::ADMIN;
            else if (power == "USER") newPower = Privilege::USER;
            else if (power == "GUEST") newPower = Privilege::GUEST;
            else {
                Logger::log("Invalid privilege level. Use ADMIN/USER/GUEST.", "ERROR");
                return;
            }

            target->second.upgradePower(newPower);
            std::string msg = uname + " has been promoted to " + power + ".";
            Logger::log(msg, "INFO");

            
        },
        Privilege::ADMIN,
        "PRIV EXC"
    };
    commandTable["DEMOTE"]={
        [this](int fd,const std::vector<std::string>& token){
            if (token.size() < 3) {
                Logger::log("Usage: DEMOTE <username> <GUEST/USER>", "ERROR");
                return;
            }

            auto uname = token[1];
            auto power = token[2];

            auto it = client.find(fd);
            if (it == client.end()) {
                Logger::log("Unable to Find User. " , "ERROR");
                return;
            }
            ClientInfo& info = it->second;
            Hut* hut = info.hut;
            if (!hut || !hut->isLoggedIn()) {
                Logger::log("You must be logged in to promote others.", "ERROR");
                return;
            }

            auto target = huts.find(uname);
            if (target == huts.end()) {
                Logger::log("User not found.", "ERROR");
                return;
            }
            Privilege newPower;
            
            if (power=="USER") {
                if(target->second.getPower()==Privilege::ADMIN){
                    Logger::log("Cannot demote an ADMIN to USER. ", "ERROR");
                    return;
                }
                newPower=Privilege::USER;
                target->second.upgradePower(newPower);
            } 
            else if(power=="GUEST") {
                if (target->second.getPower()==Privilege::ADMIN){
                    Logger::log("Cannot demote an ADMIN to GUEST. ", "ERROR");
                    return;
                }
                newPower= Privilege::GUEST;
                target->second.upgradePower(newPower);
            } else {
                Logger::log("Invalid privilege level. Use USER/GUEST.", "ERROR");
                return;
            }
        },
        Privilege::ADMIN,
        "Demote CMD"
    };

    commandTable["UPLOAD"]={
        [this](int fd,const std::vector<std::string>& token){
            if (token.size()<4) {
                Logger::log("Usage UPLOAD <filename> <size>", "ERROR");
                return;
            }
            //reciveData(fd,string& buff)
            std::string fname = token[2];
            size_t fsize = std::stoul(token[3]);
            
            std::string fbuffer("");
            fbuffer.reserve(fsize);

            while (fbuffer.length()<=fsize) {
                std::string chunk;
                if (!reciveData(fd, chunk)) {
                    Logger::log("Unable to Recive the file ", "ERROR");
                    fbuffer.clear();
                    continue;
                }
                fbuffer.append(chunk);   
            }

            filemanagement.saveFile(fname, fbuffer, Privilege::USER);
            Logger::log("File upload complete: " + fname, "INFO");

        },
        Privilege::ADMIN,
        "Uploads file to the server"
    };

    commandTable["DOWNLOAD"]={
        [this](int fd,const std::vector<std::string>& token){
            if (token.size()<3) {
                Logger::log("Insufficient Arguments: DOWNLOAD <filename>", "ERROR");
                return;
            }
            
            std::string filec("");
            if(!filemanagement.readFile(token[2], filec)){
                Logger::log("Unable to read file: " + token[2] , "ERROR");
            }
            for (size_t s= 0 ; s<filec.size();s+=100) {
                size_t chunkSize = std::min((size_t)100, filec.size() - s);
                std::string chunk;
                chunk.assign(filec,s,chunkSize);
                
                if(!sendData(fd, chunk)){
                    Logger::log("Unable to send file: " + token[2] , "ERROR");
                    break;
                }
            }

            Logger::log("Sent File "+token[2]+" To the Client. ","INFO");
        },
        Privilege::USER,
        "Download function"

    };

    commandTable["REMOVE"]={
                [this](int fd,const std::vector<std::string>& token){
            if (token.size()<3) {
                Logger::log("Insufficient Arguments: REMOVE <filename>", "ERROR");
                return;
            }
            std::string filename = token[2];
            if (!filemanagement.fileExists(filename)) {
                Logger::log("File Dosnt exist: ", "ERROR");
            
            }
            if(filemanagement.deleteFile(filename)){
               Logger::log("Removed file: "+filename, "INFO");
            }
        },
        Privilege::ADMIN,
        "Removal of File"
    };
}

bool Host::setupServer() {
    // Init server_socket member , bind and setup non blocking state , Init epoll_fd
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        Logger::log("Socket Creation Failed", "ERROR");
        return false;
    }
    struct sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(server_port);       

    if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        Logger::log("Bind failed", "ERROR");
        return false;
    }
    std::string message = "Server socket created on port " + std::to_string(server_socket);
    Logger::log(message, "INFO");

    int flags = fcntl(server_socket, F_GETFL, 0);
    fcntl(server_socket, F_SETFL, flags | O_NONBLOCK);

    epoll_fd = epoll_create1(0); 
    if (epoll_fd == -1) {
        Logger::log("Failed to create epoll instance", "ERROR");
    }

    struct epoll_event ev{};
    ev.events = EPOLLIN ;    
    ev.data.fd = server_socket; 
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &ev) == -1) {
        Logger::log("Failed to add fd to epoll", "ERROR");
    }
    return true;
}


bool Host::runServer(){
    struct epoll_event events[10]; 
    int n = epoll_wait(epoll_fd, events, 10, 1000); 

    for (int i = 0; i < n; ++i) {

        int fd = events[i].data.fd;

        if (fd == server_socket) {
            std::cout << ".";
            acceptClient();
            for (auto &pair : client) {
                int fd = pair.first;
                ClientInfo &info = pair.second;
                if (info.metadata == "UNUSED") {
                    if (addFdToEpoll(fd, EPOLLIN)) {
                        info.metadata = "ADDED_IN_EPOLL";
                        Logger::log("Added the New Socket to EPOLL.  " + std::to_string(fd), "INFO");
                    }
                }
            }
            continue;
        }
        
        if (events[i].events & EPOLLIN) {
            std::cout << "Socket " << events[i].data.fd << " is ready to read\n";

            char buffer[1024];
            int bytes = read(fd, buffer, sizeof(buffer));
            if (bytes <= 0) {
                close(fd);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                Logger::log("Client disconnected: " + std::to_string(fd), "INFO");
            }else if(bytes>0) {
                std::string client_msg(buffer,bytes);
                std::vector<std::string> token_list = Lexer::lexer(client_msg);
                handleCommand(fd,token_list);
            }


        }
    }
    return true;
}

bool Host::addFdToEpoll(int fd, uint32_t events) {
    struct epoll_event ev{};
    ev.events = events;  
    ev.data.fd = fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        Logger::log("Failed to add fd to epoll: " + std::to_string(fd), "ERROR");
        return false;
    }
    return true;
}


bool Host::acceptClient() {
    struct sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = accept(server_socket, (struct sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        Logger::log("Accept failed", "ERROR");
        return false;
    }

    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    struct epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = client_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

    Logger::log("Client Accepted: " + std::to_string(client_fd), "INFO");
    
    ClientInfo clientInfo;
    clientInfo.metadata = "UNUSED";
    client[client_fd]=clientInfo;

    return true;
}

void Host::handleCommand(int fd, const std::vector<std::string>& tokens){
    if(tokens.size()<=0){
        return;
    }
    auto it = commandTable.find(tokens[0]);
    if(it==commandTable.end()){
        Logger::log("Command Not Found. ", "ERROR");
        return;
    }
    Command &cmd = it->second;
    Hut* hut = client[fd].hut;
    if (!hut) {
        Logger::log("You must log in first.", "ERROR");
        return;
    }
    if (static_cast<int>(hut->getPower()) < static_cast<int>(cmd.requiredPrivilege)) {
        Logger::log("Required Higher Privilage. ", "ERROR");
        return;
    }
    cmd.action(fd,tokens);
}

bool Host::sendData(int clientSocket, const std::string& data) {

    const char* buffer = data.c_str();
    if(send(clientSocket,buffer,data.size(),0)<data.size()){
        Logger::log("Unable to Send full bytes on Socket: " + std::to_string(clientSocket), "ERROR");
        return false;
    }
    return true;
}


bool Host::reciveData(int clientSocket, std::string& buffer) {
    char buf[1024];
    ssize_t received = recv(clientSocket, buf, sizeof(buf), 0);

    if (received <= 0) {
        Logger::log("Unable to Recive on Socket: " + std::to_string(clientSocket), "ERROR");
        return false;
    }
    buffer.assign(buf, received);
    return true;
}