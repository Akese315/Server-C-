#include "Supervisor.hpp"
std::unordered_map<std::string, Supervisor *> Supervisor::objectList;

ThreadPool<Task, 10> Supervisor::threadPool([](Task task)
                                            { Supervisor::receive_async_function(task); });

Supervisor::Supervisor(std::string name)
{
    this->objectList.insert({name, this});
    std::cout << this << " address of " << name << std::endl;
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
            Console::print_warning(message);
            break;
        }
    }
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