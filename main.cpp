#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <csignal>
#include "Console.hpp"
#include "MyServer.hpp"
#include "ThreadPool.hpp"
#include "App.hpp"

MyServer * server;
App * app;
bool loop;

void cleanup(int signum)
{
    std::string quit = "";
    std::cout <<"\n";
    Console::printWarning("are you sure you want to leave? (quit)");
    std::getline(std::cin, quit);
    if(quit == "quit")
    {
        exit(signum);
    }else
    {
        Console::printInfo("abort connard");
    }        
}

void cleanup()
{
    
    server->stop();
    app->stop();

    delete app;
    delete server;
    loop = false;
    Log::closeFile();
}



int main()
{

    
    server = new MyServer("server");
    app =new  App("app");
   
    struct sigaction siga;
    siga.sa_handler = cleanup;                                                                                                                                                                                                                                                                   
    sigaction(SIGINT,&siga,NULL);
    atexit(cleanup);

    unsigned short int port =  3000; 

    Listener listener = server->createListener(MyServer::TCP, port,-1);
    Listener listener2 = server->createListener(MyServer::UDP, port,-1);
    server->addListener(listener);
    server->addListener(listener2);

    server->start();

    std::string command;

    loop = true;

    while(loop)
    {   
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return 0;
};

