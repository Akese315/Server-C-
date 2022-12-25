#include "Client.h"

Client::Client(uint32_t address, ushort port)
{
    this->address = address;
    this->currentData = new Datacache();
    this->port = port;
}

std::string Client::getIP()
{
    char ip[4];
    struct in_addr addr{};
    addr.s_addr = this->address;
    return inet_ntoa(addr);
}

ushort Client::getPort()
{
    return this->port;
}

ushort Client::getDataCacheState()
{
    return this->currentData->state;
}

ushort Client::getDataCacheEvent()
{
    return this->currentData->event;
}

void Client::setDataCacheEvent(ushort event)
{
    this->currentData->event = event;
}

void Client::setDataCacheData(char*data, ushort dataLen)
{
    this->currentData->data = data;
    this->currentData->dataLen = dataLen;
}

ushort Client::getDataCacheLen()
{
    return this->currentData->dataLen;
}

void Client::setPreviousData()
{
    this->previousData = this->currentData;
    delete this->currentData;
    this->currentData = new Datacache;
}

void Client::setDataCacheState(ushort state)
{
    this->currentData->state = state;
};
