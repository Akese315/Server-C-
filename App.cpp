#include "App.hpp"
#include <thread>

App::App(std::string name) : Supervisor(name)
{
    this->myThread = std::thread(&App::loop, this);
    this->loopBool = true;
    this->isRunning = true;
}

void App::loop()
{
    while (this->loopBool)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        Task task{};
        task.flag = 2;
        task.destination = "server";
        // this->doTask(task);
    }
};

void App::async_function(Task task)
{
    Logger::add_logs("hey this is the async function of app", LogLevel::INFO);
}

void App::stop()
{
    this->isRunning = false;
    if (this->myThread.joinable())
    {
        loopBoolMutex.lock();
        this->loopBool = false;
        loopBoolMutex.unlock();
        this->myThread.join();
    }
    Logger::add_logs("App is stopped");
}

App::~App()
{
    if (this->isRunning)
    {
        this->stop();
        Logger::add_logs("App is stopped");
    }
}
