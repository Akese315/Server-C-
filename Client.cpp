#include "Client.hpp"


Client::Client(uint32_t address, ushort port, int socket)
{
    this->address = address;
    this->port = port;
    this->socket = socket;
    this->currentData = new Datacache();
}

Client::~Client()
{
    //condition variable here before closing
    //check if tasks is == 0
    close(this->socket);
}

std::string Client::getIP()
{
    struct in_addr addr{};
    addr.s_addr = this->address;
    return inet_ntoa(addr);
}
int Client::getSocket()
{
    return this->socket;
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

void Client::sendData(const char* data, size_t len)
{
    
    this->fdmutex.lock();
    int bytes = send(this->socket, data ,len,MSG_DONTWAIT);
    this->fdmutex.unlock();
    if((size_t)bytes != len)
    {
        std::string error = "Message length is : "+std::to_string(len)+" but sent : "+std::to_string(bytes)+" bytes";
        Console::printError(error);
    }
};

int Client::receiveData(void * data)
{
    int bytes = recv(this->socket,data,sizeof(data),MSG_DONTWAIT);
    return bytes;
}