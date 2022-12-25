#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <iostream>
#include <arpa/inet.h>


typedef struct Datacache
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
        Client(uint32_t address, ushort port);
        std::string getIP();
        ushort getPort();
        ushort getDataCacheState();
        ushort getDataCacheEvent();
        ushort getDataCacheLen();
        void setDataCacheState(ushort state);
        void setDataCacheEvent(ushort event);
        void setDataCacheData(char* data, ushort dataLen);
        void setPreviousData();
        
    private:
        uint32_t address;
        ushort port;
        struct Datacache *previousData;
        struct Datacache *currentData;
    

};