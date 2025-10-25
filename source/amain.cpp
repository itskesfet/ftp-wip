
#include "../include/host.hpp"
#include "../include/logfoo.hpp"

#include <unistd.h> 

#define SERVER_PORT 9009

int main(int argc , char* argv[]){

    Host server(SERVER_PORT);
    server.initCommandTable();
    if(!server.setupServer()){
        Logger::log("setupServer Failed","[ERROR]");
        return -1;
    }
    while (true) { 
        server.runServer(); 
        usleep(1000);
    }
    return 0;
}