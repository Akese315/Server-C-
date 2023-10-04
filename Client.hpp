#pragma once
#include <stdio.h>
#include <string>
#include <netinet/in.h>
#include<unistd.h>
#include <mutex>
#include <arpa/inet.h>
#include "Console.hpp"

struct Datacache
{
    const static ushort COMPLETED = 0;
    const static ushort PENDING = 1;
    const static ushort NOT_USE = 2;
    char * data;
    ushort  dataLen;
    ushort state = Datacache::NOT_USE;
    ushort event;
};

class Client
{
    public:
        Client(uint32_t adress, ushort port,int socket);
        ~Client();
        std::string getIP();
        int getSocket();
        ushort getPort();
        ushort getDataCacheState();
        ushort getDataCacheEvent();
        ushort getDataCacheLen();
        void setDataCacheState(ushort state);
        void setDataCacheEvent(ushort event);
        void setDataCacheData(char* data, ushort dataLen);
        void setPreviousData();

        void sendData(const char * data, size_t len);
        int receiveData(void *data);

    private:
        uint32_t address;
        ushort port;
        ushort tasks;
        int socket;
        std::mutex fdmutex;
        struct Datacache * previousData;
        struct Datacache * currentData;
};