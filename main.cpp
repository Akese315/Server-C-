#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <csignal>
#include <atomic>
#include "Logger.hpp"
#include "MyServer.hpp"
#include "ThreadPool.hpp"
#include "App.hpp"

MyServer *server;
App *app;
std::atomic<bool> keep_running(true);

void cleanup()
{
    Logger::add_logs("cleanup function called", LogLevel::PASS);
    server->stop();
    app->stop();
    keep_running = false;
}

void signal_handler(int signum)
{
    std::string quit = "";
    std::cout << "\n";
    Logger::add_logs("are you sure you want to leave? (quit)", LogLevel::WARNING);
    std::getline(std::cin, quit);
    if (quit == "quit")
    {
        cleanup();
    }
    else
    {
        Logger::add_logs("abort", LogLevel::CRITICAL);
    }
}

int main()
{

    struct sigaction siga;
    siga.sa_handler = signal_handler;
    sigaction(SIGINT, &siga, NULL);

    unsigned short int port = 3000;
    Logger logger("log.txt");
    server = new MyServer("server");
    app = new App("app");
    Listener listener = server->create_listener(MyServer::TCP, port, -1);
    Listener listener2 = server->create_listener(MyServer::UDP, port, -1);
    server->add_listener(listener);
    server->add_listener(listener2);

    server->start();

    std::string command;

    while (keep_running)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    delete server;
    delete app;
    return 0;
};
