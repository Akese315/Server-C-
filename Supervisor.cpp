#include "Supervisor.hpp"
std::unordered_map<std::string,Supervisor*> Supervisor::objectList;

ThreadPool<Task,10> Supervisor::threadPool([](Task task) {
    Supervisor::receiveAsyncFunction(task);
});

Supervisor::Supervisor(std::string name)
{
    this->objectList.insert({name,this});
    std::cout << this <<" address of "<< name<<std::endl;   
}



Supervisor::~Supervisor()
{
    for(std::pair<std::string,Supervisor*> object : objectList)
    {
        if(object.second == this)
        {
            std::stringstream ss;
            ss << this;
            std::string message = "objet "+object.first+" : "+ss.str()+" has been removed";
            Console::printWarning(message);
            objectList.erase(object.first);
            break;
        }
    }
}

void Supervisor::receiveAsyncFunction(Task task)
{


    for(std::pair<std::string,Supervisor*> object : objectList)
    {
        if(task.destination == object.first)
        {
            object.second->asyncFunction(task);
        }
    }
}

void Supervisor::doTask(Task task)
{
    Supervisor::threadPool.send(task);
}