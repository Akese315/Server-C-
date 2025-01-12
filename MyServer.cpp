#include "MyServer.hpp"

MyServer::MyServer(std::string name) : Server(name)
{
}

MyServer::~MyServer()
{
}

void MyServer::receive_task(std::shared_ptr<Client> client, uint32_t flagTask)
{
    if (flagTask == MyServer::MESSAGE)
    {
        char tempBuffer[1024];
        int byte = client->receiveData(tempBuffer, 1024);
        tempBuffer[byte] = '\0';
        std::cout << client->get_adress_str() << " said : " << tempBuffer << std::endl;
    }
    if (flagTask == MyServer::CONNECTION)
    {
        Console::print_success("new connection : " + std::to_string(client->getSocket()));
        client->sendData("WELCOME_MESSAGE", 21);
    }
    if (flagTask == MyServer::CLIENT_DISCONNECTION)
    {
        Console::print_info(client->get_adress_str() + " has Flags::DISCONNECTED.");
    }
    if (flagTask == 2)
    {
        std::cout << "app" << std::endl;
    }
}