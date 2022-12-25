#pragma once
#include <vector>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bitset>
#include <sys/types.h>

#include "PlayerManager.h"
#include "EventData.h"

typedef struct Listener
{
    int socket;
    unsigned short type;
    int timeout;
    unsigned short port;
};



class Server
{

private:
    std::vector<Listener> ListenerArray;

    const static unsigned short int MAX_FDS_SIZE = 300;
    const static unsigned short int MAX_PACKET_SIZE = 1024;        
    const char WELCOME_MESSAGE[20] = "Welcome to our game";

    const static ushort TCP_SOCKET = 3;
    const static ushort UDP_SOCKET = 2;
    const static ushort TCP_LISTENER = 1;

    struct pollfd fds[300];
    int activeFds = 0;
    PlayerManager * playerManager;
    bool canStart = true;

    void (*onConnectionCallback)();
    void (*onReceiveCallback)(char* buffer);
    void (*onDisconnectCallback)();

    void getError(const char* where);
    void Disconnected(int *fd);
    void purgeFDs();    
    void addFD(int fd);
    void checkFDStatus(int fdr);
    void Send(Player * player, int fd, ushort type, const char* buffer);
    void Receive(Player * player, int *fd, ushort type);
    ushort getEvent(int * fd, ushort type);
    int getContent(int *fd, ushort type,  char * buffer, ushort contentLen);
    Player* retrieveClientUDP(int listener);
    Player* retrieveClientTCP(int listener);
    ushort checkFdType(int fd);
    void sendDATA();
    bool addNewConnection(int fd);
    void closeAllConnections();
    Player * createClient(sockaddr_in socketParam);
 
public:
    const static short int TCP = 1;
    const static short int UDP = 2;

    void init();
    void serverStart();
    void addListener(Listener listener);
    void removeListener(int position);
    void addClientManager(PlayerManager * playerManager);
    Listener createListener(short int type, unsigned short int port, short timeout);
    void setOnConnection(void (* callback)());
    void setOnReceive(void(* callback)(char* buffer));
    void setOnDisconnect(void(* callback)());    

};


