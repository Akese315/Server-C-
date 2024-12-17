#pragma once
#include <unordered_map>
#include <string>
#include "Console.hpp"
#include "ThreadPool.hpp"
#include <functional>
#include <iostream>
#include <sstream>

class Supervisor
{
public:
    Supervisor(std::string name);
    ~Supervisor();

protected:
    void doTask(Task task);
    virtual void asyncFunction(Task task){};

private:
    static ThreadPool<Task, 10> threadPool;
    static void receiveAsyncFunction(Task task);
    static std::unordered_map<std::string, Supervisor *> objectList;
};