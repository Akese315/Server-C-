#pragma once
#include "Supervisor.hpp"
#include <chrono>

class App : public Supervisor
{
public:
    ~App();
    App(std::string name);
    void stop();

private:
    std::thread myThread;
    std::mutex loopBoolMutex;
    bool loopBool;
    bool isRunning;

    void loop();
    void async_function(Task task) override;
};