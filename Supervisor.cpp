#include "Supervisor.hpp"
std::unordered_map<std::string, Supervisor *> Supervisor::objectList;

ThreadPool<Task, 10> Supervisor::threadPool([](Task task)
                                            { Supervisor::receive_async_function(task); });

Supervisor::Supervisor(std::string name)
{
    this->objectList.insert({name, this});
    std::stringstream ss;
    ss << this;
    Logger::add_logs("objet " + name + " : " + ss.str() + " has been created");
}

Supervisor::~Supervisor()
{
    for (std::pair<std::string, Supervisor *> object : objectList)
    {
        if (object.second == this)
        {
            std::stringstream ss;
            ss << this;
            objectList.erase(object.first);
            std::string message = "objet " + object.first + " : " + ss.str() + " has been removed";
            Logger::add_logs(message);
            break;
        }
    }
}

int Supervisor::get_running_workers()
{
    return threadPool.get_running_workers();
}

int Supervisor::get_max_workers()
{
    return threadPool.get_max_workers();
}

void Supervisor::receive_async_function(Task task)
{

    for (std::pair<std::string, Supervisor *> object : objectList)
    {
        if (task.destination == object.first)
        {
            object.second->async_function(task);
        }
    }
}

void Supervisor::do_task(Task task)
{
    Supervisor::threadPool.send(task);
}