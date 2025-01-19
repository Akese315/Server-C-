#pragma once
#include <unordered_map>
#include <string>
#include "Logger.hpp"
#include "ThreadPool.hpp"
#include <functional>
#include <iostream>
#include <sstream>

class Supervisor
{
public:
    Supervisor(std::string name);
    int get_running_workers();
    int get_max_workers();
    ~Supervisor();

protected:
    void do_task(Task task);
    virtual void async_function(Task task) {};

private:
    static ThreadPool<Task, 10> threadPool;
    static void receive_async_function(Task task);
    static std::unordered_map<std::string, Supervisor *> objectList;
};