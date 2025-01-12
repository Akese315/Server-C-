#include "Client.hpp"

Client::Client(uint32_t address, ushort port, int socket)
{
    this->isActive = true;
    this->address = address;
    this->port = port;
    this->socket = socket;
    this->currentData = new Datacache();
}

Client::~Client()
{
    // condition variable here before closing
    // check if tasks is == 0
    close(this->socket);
}

std::string Client::get_adress_str()
{
    struct in_addr addr
    {
    };
    addr.s_addr = this->address;
    std::string adress = std::string(inet_ntoa(addr)) + ":" + std::to_string(this->port);
    return adress;
}
int Client::getSocket()
{
    return this->socket;
}
ushort Client::get_port()
{
    return this->port;
}
uint32_t Client::get_ip()
{
    return this->address;
}

ushort Client::getDataCacheState()
{
    return this->currentData->state;
}

ushort Client::getDataCacheEvent()
{
    return this->currentData->event;
}

void Client::set_active(bool active)
{
    this->isActive = active;
}

void Client::setDataCacheEvent(ushort event)
{
    this->currentData->event = event;
}

void Client::setDataCacheData(char *data, ushort dataLen)
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

void Client::sendData(const char *data, size_t len)
{

    try
    {
        if (!this->isActive)
        {
            Console::print_error("Client is not active");
            return;
        }
        std::unique_lock<std::shared_mutex> lock(this->fdmutex);
        this->onSendData(data, len);
    }
    catch (...)
    {
        Console::print_error("Error while sending");
        throw std::runtime_error("Error while sending");
    }
};

int Client::receiveData(void *data, size_t len)
{
    std::shared_lock<std::shared_mutex> lock(this->fdmutex);
    return onReceiveData(data, len);
}

void Client::onSendData(const char *data, size_t len)
{
    int bytes = send(this->socket, data, len, MSG_DONTWAIT);
    if ((size_t)bytes != len)
    {
        std::string error = "message length is : " + std::to_string(len) + " but sent : " + std::to_string(bytes) + " bytes";
        Console::print_error(error);
    }
}

int Client::onReceiveData(void *data, size_t len)
{
    int bytes = recv(this->socket, data, len, MSG_DONTWAIT);
    return bytes;
}