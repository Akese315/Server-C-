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
    void do_task(Task task);
    virtual void async_function(Task task) {};

private:
    static ThreadPool<Task, 10> threadPool;
    static void receive_async_function(Task task);
    static std::unordered_map<std::string, Supervisor *> objectList;
};