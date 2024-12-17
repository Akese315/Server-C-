#pragma once
#include "Server.hpp"

class MyServer : public Server<Client>
{
public:
    // chose a name to recognize
    MyServer(std::string name);
    void receiveTask(Client *client, int flagTask) override;
    

    ~MyServer();
};