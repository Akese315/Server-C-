#include "Server.h"
#include "PlayerManager.h"

void receiveCallback(char * buffer)
{
    std::cout <<"received → "<< buffer<<std::endl;
}

void connectCallback()
{
    std::cout <<"connect callback\n";
}

void disconnectCallback()
{
    std::cout <<"user is disconnected\n";
}

int main()
{
    unsigned short int port = 20;    
    Server server;
    PlayerManager *playerManager = new PlayerManager();

    Listener listener = server.createListener(Server::TCP, port,-1);
    Listener listener2 = server.createListener(Server::UDP, port,-1);
    server.addListener(listener);
    server.addListener(listener2);
    server.addClientManager(playerManager);
    server.init();

    server.setOnConnection(&connectCallback);
    server.setOnDisconnect(&disconnectCallback);
    server.setOnReceive(&receiveCallback);   

    server.serverStart();
    return 0;
}

