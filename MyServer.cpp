#include "MyServer.hpp"

MyServer::MyServer(std::string name) : Server(name)
{
}

MyServer::~MyServer()
{
}

void MyServer::receiveTask(Client *client, int flagTask)
{
    if (flagTask == MyServer::MESSAGE)
    {
        char tempBuffer[1024];
        int byte = client->receiveData(tempBuffer, 1024);
        tempBuffer[byte] = '\0';
        std::cout << client->getIP() << " said : " << tempBuffer << std::endl;
    }
    if (flagTask == MyServer::NEW_CONNECTION)
    {
        Console::printSuccess("new connection : " + std::to_string(client->getSocket()));
        client->sendData("WELCOME_MESSAGE", 21);
    }
    if (flagTask == MyServer::DISCONNECTED)
    {
        Console::printInfo(client->getIP() + " has disconnected.");
    }
    if (flagTask == 2)
    {
        std::cout << "app" << std::endl;
    }
}