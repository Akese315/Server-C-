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
#include <shared_mutex>

#include "Supervisor.hpp"
#include "ProcessMonitor.hpp"
#include "Client.hpp"

struct Listener
{
    int socket;
    unsigned short type;
    int timeout;
    unsigned short port;
};

template <typename T = Client>
class Server : public Supervisor
{

public:
    /**
     * @brief Constructor
     * This function will create a server
     *
     * @param name the name of the server (used for the supervisor and by you to identify whom to send task)
     * @return void
     */

    Server(std::string name) : Supervisor(name)
    {
        this->loopBool = true;
        this->isRunning = true;

        if (pipe(this->pipefd) == -1)
        {
            getError("pipe");
        }
    };
    ~Server()
    {
        this->closeAllConnections();
        if (this->isRunning)
        {
            this->stop();
        }
    };

    /**
     *
     * @brief Ban a client
     * This function will remove the client from the epoll and close the socket
     *
     * @param client the client to ban
     * @return void
     */

    void bansmn(T &client)
    {
        this->interrupt_epoll_wait(Server::BAN, client.getSocket());
    };

    /**
     *
     * @brief Disconnect a client
     * This function will remove the client from the epoll and close the socket
     *
     * @param client the client to disconnect
     * @return void
     */

    void disconnectsmn(T &client)
    {
        this->interrupt_epoll_wait(Server::DISCONNECTED, client.getSocket());
    };

    /**
     *
     * @brief Restart the server
     * This function will restart the server
     * It will stop the server and start it again
     *
     * @return void
     */

    void restart()
    {
        if (this->isRunning)
        {
            this->stop();
        }
        this->loopBool = true;
        this->isRunning = true;
        this->myThread = std::thread(&Server<T>::loop, this);
    };

    /**
     *
     * @brief Broadcast a message to all clients
     * This function will send a message to all clients
     *
     * @param data the data to send
     * @param len the length of the data
     * @return void
     */

    void broadcastsmth(const char *data, size_t len)
    {
        for (auto pair : this->socketClientMap)
        {
            pair->second->sendData(data, len);
        }
    };

    /**
     *
     * @brief Find a client by its socket
     * This function will return a client by its socket
     *
     * @param socket the socket of the client
     * @return T* the client
     */

    T *findClient(int socket)
    {
        auto it = this->socketClientMap.find(socket);
        if (it == this->socketClientMap.end())
        {
            return nullptr;
        }
        else
        {
            return it->second;
        }
    }
    /**
     *
     * @brief Find a client by its address
     * This function will return a client by its address
     *
     * @param addr the address of the client
     * @return T* the client
     */

    T *findClient(uint32_t addr)
    {
        auto it = this->addrPlayerMap.find(addr);
        if (it == this->addrPlayerMap.end())
        {
            return nullptr;
        }
        else
        {
            return &it->second;
        }
    }

    /**
     * @brief Stop the server
     * This function will stop the server by sending a stop flag to the epoll through the pipe
     * It will also join the thread
     * @return void
     */
    void stop()
    {
        this->isRunning = false;
        this->interrupt_epoll_wait(Server::STOP, 0);
        if (this->myThread.joinable())
        {
            this->myThread.join();
        }
        Console::printWarning("Server is stopped");
    };

    /**
     * @brief Receive a task
     * This function will receive a task from the client
     *
     * @param client the client that sent the task
     * @param taskFlag the flag of the task
     * @return void
     */

    virtual void receiveTask(T *client, int taskFlag) {};

    /**
     * @brief On disconnect
     * This function will be called when a client disconnects
     * You can override this function to add your own behavior
     */
    virtual void onDisconnect(T *client) {};

    /**
     * @brief Start the server
     * This function will start the server
     * It will create the epoll and the listeners
     * It will also start the thread
     * @return void
     */
    void start()
    {
        Console::printSuccess("Server is starting...");
        bool can_start = init();
        if (!can_start)
        {
            Console::printError("Can't start.");
            return;
        }
        Console::printSuccess("Server started :)");

        for (size_t i = 0; i < ListenerArray.size(); i++)
        {
            listen(ListenerArray[i].socket, 5);
        }
        this->myThread = std::thread(&Server<T>::loop, this);
    };

    /**
     * @brief Create a listener
     * This function will create a listener
     *
     * @param type the type of the listener (TCP or UDP)
     * @param port the port of the listener
     * @param timeout the timeout of the listener
     * @return Listener the listener
     */

    Listener createListener(uint16_t type, uint16_t port, uint16_t timeout)
    {
        Listener listener = {};
        listener.port = port;
        listener.type = type;
        listener.timeout = timeout;

        if (type == Server<T>::TCP)
        {
            listener.socket = socket(AF_INET, SOCK_STREAM, 0);
        }
        else if (type == Server<T>::UDP)
        {
            listener.socket = socket(AF_INET, SOCK_DGRAM, 0);
        }
        if (listener.socket == -1 || listener.socket == 0)
        {
            getError("create listener");
        }
        Console::printInfo("socket created : " + std::to_string(listener.socket));
        return listener;
    };

    /**
     * @brief Interrupt epoll wait
     * This function will interrupt the epoll wait and it will use a shared mutex
     *
     * @param flag the flag to send
     * @param fd the file descriptor
     * @return int
     */

    int interrupt_epoll_wait(int flag, int fd)
    {
        uint32_t message = 0;
        encode_interrupt_wait_message(message, flag, fd);
        std::shared_lock lock(this->pipe_shared_mutex);
        if (write(fd, &message, 4) == -1)
        {
            perror("write");
            return -1;
        }
        return 0;
    };

    /**
     * @brief Add a listener
     * This function will add a listener to the server
     *
     * @param listener the listener to add
     * @return void
     */

    void addListener(Listener listener)
    {
        ListenerArray.push_back(listener);
    };

    /**
     * @brief Remove a listener
     * This function will remove a listener from the server
     *
     * @param position the position of the listener to remove
     * @return void
     */

    void removeListener(int position)
    {
        ListenerArray.erase(ListenerArray.begin() + position);
    };

    const static uint16_t TCP = 1;
    const static uint16_t LISTENER = 1;
    const static uint16_t UDP = 2;

    // BASIC FLAG
    const static uint16_t NEW_CONNECTION = 3;
    const static uint16_t DISCONNECTED = 4;
    const static uint16_t MESSAGE = 5;
    const static uint16_t BAN = 6;
    const static uint16_t STOP = 0;
    // BASIC FLAG

    const static uint16_t MAX_EVENTS_SIZE = 1023; // Determine the size of a chunk of events, independant of the number of users
    const static uint16_t MAX_USERS = 65535;      // Determine the maximum number of users

private:
    struct epoll_event events[MAX_EVENTS_SIZE];
    std::vector<Listener> ListenerArray;
    std::unordered_map<uint32_t, T *> addrBanMap;
    std::unordered_map<int, T *> socketClientMap;
    int activeEvents = 0;
    int epollfd;
    int pipefd[2];
    std::vector<int> fds;

    // SERVER

    std::condition_variable cond;
    std::mutex loopBoolMutex;
    std::shared_mutex pipe_shared_mutex;
    std::thread myThread;
    bool loopBool;
    bool isRunning;

    // méthodes
    void getError(const char *where)
    {
        std::string error = strerror(errno) + (std::string) " in " + where;
        Console::printError(error);
    };

    bool hasEvent(uint32_t events)
    {
        return (events == 0) ? false : true;
    };

    bool isPipe(int fd)
    {
        return (fd == pipefd[0]) ? true : false;
    };

    bool isErrorEvent(uint32_t events)
    {
        return (events & EPOLLERR) ? true : false;
    };

    bool isReadEvent(uint32_t events)
    {
        return (events == EPOLLIN || events == (EPOLLIN | EPOLLET));
    }

    bool isListener(int fd)
    {
        for (size_t j = 0; j < ListenerArray.size(); j++)
        {
            if (ListenerArray[j].socket != fd)
            {
                continue;
            }
            if (ListenerArray[j].type != Server<T>::TCP)
            {
                continue;
            }
            return true;
        }
        return false;
    }

    bool check_file_descriptor(int fdr)
    {
        ProcessMonitor pm("check_file_descriptor");

        for (int i = 0; i < activeEvents && fdr > 0; i++)
        {
            int fd = events[i].data.fd;
            if (!isReadEvent(events[i].events))
            {
                std::cout << " Revents : " << events[i].events << ", fd : " << fd << std::endl;
                getError("revents");
                continue;
            }
            if (!hasEvent(events[i].events))
            {
                continue;
            }
            if (isPipe(fd))
            {
                if (!processPipeMessage(fd))
                {
                    return false;
                }
                continue;
            }
            if (isListener(fd))
            {
                new_connection(fd);
                fdr--;
                continue;
            }

            if (this->isDisconnected(fd))
            {

                this->disconnect(fd);
                this->sendTask(std::string("server"), std::string("server"), Server<T>::DISCONNECTED, fd);
            }
            else
            {
                this->sendTask(std::string("server"), std::string("server"), Server<T>::MESSAGE, fd);
            }
            fdr--;
        }
        return true;
    };

    bool processPipeMessage(uint32_t fd)
    {
        uint32_t message;
        uint16_t flag, fd_check;
        if (read(fd, &message, 4) == -1)
        {
            getError("read");
            return false;
        }
        decode_interrupt_wait_message(message, flag, fd_check);

        if (flag == Server<T>::STOP)
        {
            return false;
        }
        if (flag == Server<T>::BAN)
        {
            // to do
        }
        if (flag == Server<T>::DISCONNECTED)
        {
            disconnect(fd_check);
        }
        return true;
    }

    void disconnect(int fd)
    {
        T *client = this->findClient(fd);
        if (client == nullptr)
        {
            Console::printWarning("Client has already been disconnected");
            return;
        }
        onDisconnect(client);
        this->socketClientMap.erase(fd); // call the onDisconnect function
        remove_file_descriptor(fd);      // memove shift the events to fill the void created by the suppression of the fd
        close(fd);                       // decrement the number of active events
    };

    // virtual function to be implemented in the child class

    bool isDisconnected(int fd)
    {
        char buffer[1];
        if (recv(fd, buffer, 1, MSG_PEEK) == 0)
        {
            return true;
        }
        return false;
    };

    void encode_interrupt_wait_message(uint32_t &message, uint16_t flag, uint16_t fd)
    {
        message |= flag;
        message <<= 16;
        message |= fd;
    }

    void decode_interrupt_wait_message(uint32_t message, uint16_t &flag, uint16_t &fd)
    {
        fd = message & 0xFFFF;
        message >>= 16;
        flag = message & 0xFFFF;
    }

    int new_connection(int fd)
    {
        struct sockaddr_in socketParam
        {
        };
        socklen_t socketaddrLen = sizeof(socketParam);
        int new_fd = accept(fd, (struct sockaddr *)&socketParam, (socklen_t *)&socketaddrLen);
        if (new_fd < 0)
        {
            Console::printError("error on accept socket");
            getError("accept");
            return -1;
        }
        addFD(new_fd);
        T *client = new T(socketParam.sin_addr.s_addr, socketParam.sin_port, new_fd);
        socketClientMap.insert(std::pair<int, T *>(new_fd, client));
        this->sendTask(std::string("server"), std::string("server"), Server<T>::NEW_CONNECTION, new_fd);
        return new_fd;
    };
    void addFD(int fd)
    {
        if (activeEvents == MAX_EVENTS_SIZE)
        {
            return;
        }

        events[activeEvents].data.fd = fd;
        events[activeEvents].events = EPOLLIN | EPOLLET;
        epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &events[activeEvents]);
        fds.push_back(fd);
        activeEvents++;
    };
    void remove_file_descriptor(int fd)
    {
        if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL) == -1)
        {
            getError("D fd: ");
        }
        for (int i = 0; i < activeEvents; i++)
        {
            if (this->events[i].data.fd == fd)
            {
                memmove(&events[i], &events[i + 1], (activeEvents - i - 1) * sizeof(epoll_event));
                this->activeEvents -= 1;
            }
        }
    }

    void closeAllConnections()
    {
        for (unsigned short i = 1; i < activeEvents; i++)
        {
            close(events[i].data.fd);
        }

        close(pipefd[0]);
        close(pipefd[1]);
    };

    bool init()
    {
        if (ListenerArray.empty())
        {
            Console::printError("No listeners set");
            return false;
        }

        epollfd = epoll_create1(0);
        if (epollfd == -1)
        {
            getError("epol");
            // exit(EXIT_FAILURE);
        }

        if (fcntl(this->pipefd[0], F_SETFL, O_NONBLOCK) == -1)
        {
            getError("fcntl");
            // exit(EXIT_FAILURE);
        }

        fds.push_back(pipefd[0]);

        events[0].events = EPOLLIN | EPOLLET;
        events[0].data.fd = pipefd[0];
        Console::printInfo("pipe created : " + std::to_string(pipefd[0]));

        if (epoll_ctl(epollfd, EPOLL_CTL_ADD, pipefd[0], &events[0]) == -1)
        {
            getError("epoll_ctl");
        }

        int on = 1;
        // Set in File descriptor all listeners
        for (long unsigned int i = 0; i < ListenerArray.size(); i++)
        {
            setsockopt(ListenerArray[i].socket, SOL_SOCKET, SO_REUSEADDR, (char *)&on, (socklen_t)sizeof(on));
            fcntl(ListenerArray[i].socket, F_SETFL, O_NONBLOCK);
            events[i + 1].events = EPOLLIN | EPOLLET;
            events[i + 1].data.fd = ListenerArray[i].socket;
            if (epoll_ctl(epollfd, EPOLL_CTL_ADD, ListenerArray[i].socket, &events[i + 1]) == -1)
            {
                getError("Epoll ctl");
            }
            activeEvents++;
        }

        // Bind all listeners
        for (long unsigned int i = 0; i < ListenerArray.size(); i++)
        {

            sockaddr_in bindParams;
            bindParams.sin_family = AF_INET;
            bindParams.sin_port = htons(ListenerArray[i].port);
            inet_aton("127.0.0.1", &bindParams.sin_addr);
            int error = bind(ListenerArray[i].socket, (struct sockaddr *)&bindParams, (socklen_t)sizeof(bindParams));

            if (error != 0)
            {
                getError("bind");
                return false;
            }

            std::string info = "server bind on port : " + std::to_string(ListenerArray[i].port);
            Console::printInfo(info);
        }
        return true;
    };
    void asyncFunction(Task task) override
    {
        std::cout << task.fd << " " << task.flag << std::endl;
        auto it = this->socketClientMap.find(task.fd);
        if (it == this->socketClientMap.end())
        {
            Console::printError("Client not found in map");
            return;
        }
        T *client = it->second;
        this->receiveTask(client, task.flag);
    };

    void sendTask(std::string destination, std::string source, int flag, int fd)
    {
        Task task{};
        task.destination = destination;
        task.source = source;
        task.flag = flag;
        task.fd = fd;
        this->doTask(task);
    };

    void loop()
    {
        Console::printInfo("Listening...");
        do
        {
            int fdr = epoll_wait(epollfd, events, MAX_EVENTS_SIZE, -1);
            if (fdr < 0)
            {
                if (fdr == -1)
                {
                    if (errno == EINTR)
                    {
                        Console::printError("Intereupted system call");
                        continue;
                    }
                }
                perror("Erreur dans epoll_wait()");
                getError("epoll_wait");
                break;
            }
            if (fdr == 0)
            {
                Console::printWarning("Time out reached");
                break;
            }
            if (fdr > 0)
            {
                if (!check_file_descriptor(fdr))
                {
                    break;
                }
            }
        } while (true);
        Console::printInfo("not Listening...");
    };
};