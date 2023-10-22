#pragma once
#include <condition_variable>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <vector>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <csignal>

#include "Supervisor.hpp"
#include "Client.hpp"


struct Listener
{
    int socket;
    unsigned short type;
    int timeout;
    unsigned short port;
};

template<typename T = Client>
class Server: public Supervisor
{


    public:


        Server(std::string name):Supervisor(name)
        {
            this->loopBool = true;
            this->isRunning = true;

            if(pipe(this->pipefd)==-1)
            {
                getError("pipe");
            }
        };
        ~Server()
        {
            this->closeAllConnections();
            if(this->isRunning)
            {
                this->stop();
            }
        };
        
        virtual void bansmn(T* client)
        {
                          
        };
        void shutdown();
        void restart()
        {
            this->loopBool = true;
            this->isRunning = true;
            this->myThread = std::thread(&Server<T>::loop, this);
        };
        void broadcastsmth(const char * data, size_t len)
        {
            for(auto pair : this->socketPlayerMap)
            {
                pair->second->sendData(data,len);
            }
        };
        T* findClient(int socket)
        {
            auto it = this->socketPlayerMap.find(socket);
            if(it == this->socketPlayerMap.end())
            {
                return nullptr;
            }
            else
            {
                return it->second;
            }
        }
        T* findClient(uint32_t addr)
        {
            auto it = this->addrPlayerMap.find(addr);
            if(it == this->addrPlayerMap.end())
            {
                return nullptr;
            }
            else
            {
                return it->second;
            }
        }
        void stop()
        {
            this->isRunning = false;
            this->interrupt_epoll_wait();
            if(this->myThread.joinable()){
                this->myThread.join();
            }
            Console::printWarning("Server is stopped");
        };
        virtual void receiveTask(T* client, int taskFlag) {};
        void start()
        {
            Console::printSuccess("Server is starting...");
            bool can_start = init();
            if(!can_start){
                Console::printError("Can't start.");
                return;
            }
            Console::printSuccess("Server started :)");

            for(size_t i = 0; i< ListenerArray.size();i++)
            {
                listen(ListenerArray[i].socket, 5);
            }
            this->myThread = std::thread(&Server<T>::loop, this);
        };

        
        Listener createListener(short int type, unsigned short int port, short timeout)
        {
            Listener listener = {};
            listener.port = port;
            listener.type = type;
            listener.timeout = timeout;
                    
            if(type == Server<T>::TCP)
            {
                listener.socket = socket(AF_INET,SOCK_STREAM,0);
            }else if(type == Server<T>::UDP)
            {
                listener.socket = socket(AF_INET,SOCK_DGRAM,0);
            }
            if(listener.socket == -1 || listener.socket == 0)
            {
                getError("create listener");
            }
            Console::printInfo("socket created : "+ std::to_string (listener.socket));
            return listener;
        };
        void addListener(Listener listener)
        {
            ListenerArray.push_back(listener);
        };
        void removeListener(int position)
        {
            ListenerArray.erase(ListenerArray.begin()+position);
        };


        const static ushort  TCP = 1;
        const static ushort  LISTENER = 1;
        const static ushort  UDP = 2;

        //BASIC FLAG
        const static ushort  NEW_CONNECTION = 3;
        const static ushort  DISCONNECTED = 4;
        const static ushort  MESSAGE = 5;
        const static ushort  BAN = 6;
        //BASIC FLAG

        const static ushort MAX_EVENTS_SIZE = 1023;
        const char WELCOME_MESSAGE[21] = "Welcome to our game\n";

       



    private:

        struct epoll_event events[MAX_EVENTS_SIZE];
        std::vector<Listener> ListenerArray;
        std::unordered_map<uint32_t,T*> addrBanMap;
        std::unordered_map<int,T*> socketPlayerMap;
        int activeEvents = 0;
        int epollfd;
        int pipefd[2];
        std::vector<int> fds;
        
        //SERVER


        std::condition_variable cond;
        std::mutex loopBoolMutex; 
        std::thread myThread;
        bool loopBool;  
        bool isRunning;

        //méthodes
        void getError(const char* where)
        {
            std::string error = strerror(errno)+(std::string)" in "+where;
            Console::printError(error);   
        };
        bool fd_check(int fdr)
        {
            for(int i = 0; i < activeEvents && fdr > 0;i++)
            {
                if(events[i].events != EPOLLIN && events[i].events != (EPOLLIN|EPOLLET))
                {
                    std::cout <<" Revents : "<<events[i].events<< ", fd : "<<events[i].data.fd<<std::endl;            
                    getError("revents");
                    continue;
                }

                if(events[i].events == 0)
                {
                    continue;
                }
                int fd = events[i].data.fd;
                if(fd == fds[0])
                {
                    char buff[1];
                    read(events[0].data.fd,buff,1);
                    return false;
                }
                        

                for(size_t j = i; j< ListenerArray.size(); j++)
                {
                    if(ListenerArray[j].socket != fd)
                    {
                        continue;
                    }
                    if(ListenerArray[j].type != Server<T>::TCP)
                    {
                        continue;
                    }
                    new_connection(fd);
                    fdr--;
                    continue;
                }

                if(this->isDisconnected(fd))
                {
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    memmove(&events[i], &events[i+1], (activeEvents - i - 1) * sizeof(epoll_event));
                    Task task{};
                    task.fd = fd;
                    task.destination = "server";
                    task.source = "server";
                    task.flag = Server<T>::DISCONNECTED;
                    this->doTask(task);
                    activeEvents--;
                    fd = -1;
                }
                else
                {
                    Task task{};
                    task.source = "server";
                    task.destination = "server";
                    task.flag = Server<T>::MESSAGE;
                    task.fd = fd;
                    this->doTask(task);
                }
                fdr--;
            }
            return true;
        };
        bool isDisconnected(int fd)
        {
            char buffer[1];
            if(recv(fd,buffer,1,MSG_PEEK) ==0){
                return true;
            }
            else return false;
        };
        int interrupt_epoll_wait()
        {
            char dummy = 'a';
            if (write(this->pipefd[1], &dummy, sizeof(dummy)) == -1) {
                perror("write");
                return -1;
            }
            return 0;
        };
        int new_connection(int fd)
        {
            struct sockaddr_in socketParam{};
            socklen_t socketaddrLen = sizeof(socketParam);
            int newFD = accept(fd,(struct sockaddr *)&socketParam,(socklen_t *)&socketaddrLen);
            if(newFD < 0)
            {
                Console::printError("error on accept socket");
                getError("accept");
                return -1;
            }    
            addFD(newFD);
            T* client = new  T(socketParam.sin_addr.s_addr,socketParam.sin_port, newFD);
            socketPlayerMap.insert({newFD,client}); 

            Task task{};
            task.source = "server";
            task.destination = "server";
            task.flag = Server<T>::NEW_CONNECTION;
            task.fd = newFD;
            this->doTask(task);

            return newFD;
        };
        void addFD(int fd)
        {
            if(activeEvents == MAX_EVENTS_SIZE)
            {
                return;
            }

            events[activeEvents].data.fd = fd;
            events[activeEvents].events = EPOLLIN|EPOLLET;
            epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &events[activeEvents]);
            fds.push_back(fd);
            activeEvents++;
        };
        void removeFD(int fd)
        {
            if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL) == -1) {
                getError("Remove fd: ");
            }
            for(int i = 0; i<activeEvents; i ++)
            {
                if(this->events[i].data.fd == fd)
                {
                    memmove(&events[i], &events[i+1], (activeEvents - i - 1) * sizeof(epoll_event));
                    this->activeEvents -=1;
                }
            }
        }

        void closeAllConnections()
        {
            for(unsigned short i = 1; i< activeEvents; i++)
            {
                close(events[i].data.fd);
            }

            close(pipefd[0]);
            close(pipefd[1]);
        };
        bool init()
        {
            if(ListenerArray.empty())
            {
                Console::printError("No listeners set");
                return false;
            }

            epollfd = epoll_create1(0);
            if (epollfd == -1) {
                getError("epol");
                //exit(EXIT_FAILURE);
            }

            if (fcntl(this->pipefd[0], F_SETFL, O_NONBLOCK) == -1) {
                getError("fcntl");
                //exit(EXIT_FAILURE);
            }

            fds.push_back(pipefd[0]);

            events[0].events  = EPOLLIN| EPOLLET;
            events[0].data.fd  = pipefd[0];
            Console::printInfo("pipe created : "+ std::to_string (pipefd[0]));

            if (epoll_ctl(epollfd, EPOLL_CTL_ADD, pipefd[0], &events[0]) == -1) {
                getError("epoll_ctl");
            }

            int on = 1;
            //Set in File descriptor all listeners
            for(long unsigned int i = 0 ; i< ListenerArray.size();i++)
            {
                setsockopt(ListenerArray[i].socket, SOL_SOCKET,  SO_REUSEADDR,(char *)&on,(socklen_t)sizeof(on)); 
                fcntl(ListenerArray[i].socket, F_SETFL, O_NONBLOCK);
                events[i+1].events = EPOLLIN | EPOLLET;
                events[i+1].data.fd = ListenerArray[i].socket;
                if(epoll_ctl(epollfd, EPOLL_CTL_ADD, ListenerArray[i].socket, &events[i+1]) == -1)
                {
                    getError("Epoll ctl");
                }
                activeEvents++;
            }

            //Bind all listeners
            for(long unsigned int i = 0; i < ListenerArray.size();i++)
            {

                sockaddr_in bindParams;
                bindParams.sin_family =AF_INET;
                bindParams.sin_port = htons(ListenerArray[i].port);
                inet_aton("127.0.0.1",&bindParams.sin_addr);
                int error = bind(ListenerArray[i].socket,(struct sockaddr *)&bindParams,(socklen_t)sizeof(bindParams));

                if(error != 0)
                {
                    getError("bind");
                    return false;
                }

                std::string info = "server bind on port : "+std::to_string(ListenerArray[i].port);
                Console::printInfo(info);
            }
            return true;
        };
        void asyncFunction(Task task) override
        {
            auto it = this->socketPlayerMap.find(task.fd);
            if(it == this->socketPlayerMap.end())
            {
                return;
            }
            T* client = it->second;

            this->receiveTask(client,task.flag);  
            if(task.flag == Server<T>::BAN)
            {
                this->bansmn(client);
                this->removeFD(it->first);
            }  
            if(task.flag == Server<T>::DISCONNECTED|| task.flag == Server<T>::BAN)
            {
                this->socketPlayerMap.erase(it);
                delete client;
            }

        };
        void loop()
        {
            Console::printInfo("Listening..."); 
            do{
            int fdr = epoll_wait(epollfd, events, MAX_EVENTS_SIZE, -1);
            if (fdr < 0)
                {
                    if(fdr == -1)
                    {
                    if (errno == EINTR) {
                        Console::printError("interrompue par un signal");
                        continue;
                        }
                    }
                    perror("Erreur dans epoll_wait()");
                    getError("epoll_wait");
                    break;
                }
                if(fdr == 0)
                {
                    Console::printWarning("Time out reached");
                    break;
                }
                if(fdr > 0)
                {   
                    if (!fd_check(fdr))
                    {
                        break;
                    }
                }
            }while(true);
            Console::printInfo("not Listening..."); 
        };

};