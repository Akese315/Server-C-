#pragma once
#include <stdio.h>
#include <string>
#include <netinet/in.h>
#include <unistd.h>
#include <mutex>
#include <arpa/inet.h>
#include <shared_mutex>
#include "Console.hpp"

struct Datacache
{
    const static ushort COMPLETED = 0;
    const static ushort PENDING = 1;
    const static ushort NOT_USE = 2;
    char *data;
    ushort dataLen;
    ushort state = Datacache::NOT_USE;
    ushort event;
};

class Client
{
public:
    bool isActive;

    Client(uint32_t adress, ushort port, int socket);
    ~Client();
    int getSocket();
    std::string get_adress_str();
    uint32_t get_ip();
    ushort get_port();
    ushort getDataCacheState();
    ushort getDataCacheEvent();
    ushort getDataCacheLen();
    void setDataCacheState(ushort state);
    void setDataCacheEvent(ushort event);
    void setDataCacheData(char *data, ushort dataLen);
    void setPreviousData();
    void set_active(bool active);
    void sendData(const char *data, size_t len);
    int receiveData(void *data, size_t len);

private:
    uint32_t address;
    ushort port;
    ushort tasks;
    int socket;
    std::shared_mutex fdmutex;
    struct Datacache *previousData;
    struct Datacache *currentData;

protected:
    virtual int onReceiveData(void *data, size_t len);     // do not call directly this function
    virtual void onSendData(const char *data, size_t len); // do not call directly this function
};