#include "Server.h" 

void Server::init()
{
    if(ListenerArray.empty())
    {
        std::cout << "No listeners set\n";
        canStart = false;
        return;
    }

    memset(&fds,0, sizeof(&fds));

    int on =  1;

    //Set in File descriptor all listeners
    for(int i = 0; i< ListenerArray.size();i++)
    {
        setsockopt(ListenerArray[i].socket, SOL_SOCKET,  SO_REUSEADDR,(char *)&on,(socklen_t)sizeof(on)); 
        fcntl(ListenerArray[i].socket, F_SETFL, O_NONBLOCK);
        fds[i].events = POLLIN;
        fds[i].fd = ListenerArray[i].socket;
        activeFds++;
    }

    //Bind all listeners
    for(int i = 0; i < ListenerArray.size();i++)
    {
        sockaddr_in bindParams;
        bindParams.sin_family =AF_INET;
        bindParams.sin_port = htons(ListenerArray[i].port);
        inet_aton("127.0.0.1",&bindParams.sin_addr);
        int error = bind(ListenerArray[i].socket,(struct sockaddr *)&bindParams,(socklen_t)sizeof(bindParams));
        if(error != 0)
        {
            getError("bind");
            canStart = false;
            return;
        }
        std::cout << "server bind on port : "<< ListenerArray[i].port<<std::endl;
    }
    
}

Listener Server::createListener(short int type, unsigned short int port, short timeout)
{
    
    Listener listener = {};
    listener.port = port;
    listener.type = type;
    listener.timeout = timeout;
    
    if(type == Server::TCP)
    {
        listener.socket = socket(AF_INET,SOCK_STREAM,0);
    }else if(type == Server::UDP)
    {
        listener.socket = socket(AF_INET,SOCK_DGRAM,0);
    }
    if(listener.socket == -1 || listener.socket == 0)
    {
        getError("create listener");
    }
    std::cout << "socket created : " << listener.socket<<"\n";
    return listener;
}

void Server::serverStart()
{
    if(&playerManager == nullptr)
    {
        std::cout<<"can\'t start because the server has no playerManager\n";
        return;
    }
    if(!canStart)
    {
        std::cout<<"can\'t start\n";
        return;
    }
    for(int i = 0; i< ListenerArray.size();i++)
    {
        listen(ListenerArray[i].socket, 5);
    }

    std::cout << "Listening...\n"; 
    do{
        int fdr = poll(fds,activeFds,-1);
        if (fdr < 0)
        {
            getError("poll");
            break;
        }
        if(fdr == 0)
        {
            std::cout <<"Time out reached\n";
            break;
        }
        if(fdr > 0)
        {
            checkFDStatus(fdr);
        }
    }while(true);
}

void Server::checkFDStatus(int fdr)
{
    for(int i = (activeFds -1); i >= 0; i--)
    {
        if(fds[i].revents == 0)
        {
            continue;
        }

        if(fds[i].revents != POLLIN)
        {
            std::cout <<" Revents : "<<fds[i].revents<< ", fd : "<<fds[i].fd<<std::endl;            
            getError("revents");
            continue;
        }
        bool isListener = false;

        //We have to check if it's a TCP listener

        ushort type = checkFdType(fds[i].fd);
        if(type == Server::TCP_LISTENER)
        {
            addNewConnection(fds[i].fd);
            continue;
        }   

        Player * player;
        if(type == Server::UDP_SOCKET)
        {
            player = retrieveClientUDP(fds[i].fd);
        }
        else if (type == Server::TCP_SOCKET)
        {
            player = retrieveClientTCP(fds[i].fd);
        }

        Receive(player,&fds[i].fd, type);       
    }
}

ushort Server::checkFdType(int fd)
{
    //if fd is a TCP listener → return 1
    //if fd is a UDP socket → return 2
    //if fd is a TCP socket → return 3

    for(int SocketIndex = 0; SocketIndex< ListenerArray.size();SocketIndex++)
    {
        if(fd == ListenerArray[SocketIndex].socket)    
        {
            if(ListenerArray[SocketIndex].type == Server::TCP)
            {
                return 1;
            } 
            if(ListenerArray[SocketIndex].type == Server::UDP)
            {
                return 2;
            }                       
        }
        else
        {
            return 3;
        } 
    }    
}

Player* Server::retrieveClientUDP(int listener)
{
    char * buffer;
    sockaddr_in addr = {};
    recvfrom(listener,buffer,0,MSG_PEEK,(sockaddr*)&addr,(socklen_t *)sizeof(addr));//read header
    Player* player = playerManager->getPlayer(addr.sin_addr.s_addr);
    return player;
}

Player* Server::retrieveClientTCP(int listener)
{
   Player* player = playerManager->getPlayer(listener);
   return player;
}

void Server::Receive(Player* player,int *fd, ushort type)
{
    //data incoming;
    std::cout <<"data incoming\n";
    ushort state = player->getDataCacheState();
    if(state == Datacache::NOT_USE)
    {
       ushort event = getEvent(fd, type);
       if(event == 0)return;       
       std::cout << "New event : "<<event<<std::endl;   
       player->setDataCacheState(Datacache::PENDING);
       player->setDataCacheEvent(event);
    }
    if(state== Datacache::PENDING)
    {
        ushort event = player->getDataCacheEvent();
        ushort eventLen = Event::getEventByte(event);
        ushort eventLenRemaining = eventLen - player->getDataCacheLen(); 
        char buffer[eventLenRemaining];
        memset(&buffer,0, eventLenRemaining);
        int bytes = getContent(fd, type, buffer,eventLenRemaining);
        if(bytes == 0)return;  
        player->setDataCacheData(buffer,bytes);
        if(bytes == eventLenRemaining)
        {
            player->setDataCacheState(Datacache::COMPLETED);
            onReceiveCallback(buffer);
        }
    }
}

void Server::Send(Player * player, int fd, ushort type, const char * buffer)
{
    if(type == Server::TCP)
    {
        std::cout << "buffer : "<<*buffer<<"\n";
        void * newbuffer = (void*) buffer;
      
        std::cout<< "fd : " << fd<<"\n";
        std::cout << "buffer : "<< newbuffer;
        send(fd,newbuffer,(size_t) sizeof(newbuffer),0);
        std::cout <<"sent\n";        
    }else
    if(type == Server::UDP)
    {
        sockaddr_in addr = {};
        addr.sin_port = player->getPort();
        addr.sin_family = AF_INET;
        inet_aton(player->getIP().c_str(),&addr.sin_addr);
        sendto(fd,buffer,sizeof(buffer),0,(sockaddr*)&addr,(socklen_t)sizeof(addr));
    }
}

ushort Server::getEvent(int *fd, ushort type)
{
    char buffer[Event::EventLen];
    memset(buffer,0,Event::EventLen);
    if(type == Server::TCP_SOCKET)
    {
        int bytes = recv(*fd,buffer,Event::EventLen,0);
        std::cout << "bytes Received : "<<bytes<<std::endl;
        if(bytes == 0)
        {
            Disconnected(fd);
            return 0;
        }
    }
    else
    {
       int bytes = recvfrom(*fd,buffer,Event::EventLen,0,NULL, NULL);
    }
    
    std::bitset<16> third((
    std::bitset<8>(buffer[1])).to_string()+
    std::bitset<8U>(buffer[0]).to_string());
    
    return (ushort)third.to_ulong();
}

int Server::getContent(int * fd, ushort type, char * buffer, ushort contentLen)
{
    int bytes;
    if(type == Server::TCP_SOCKET)
    {
        bytes = recv(*fd,buffer,contentLen,0);
        if(bytes == 0)
        {
            Disconnected(fd);
            return 0;
        }
    }
    else
    {
       bytes = recvfrom(*fd,buffer,contentLen,0,NULL, NULL);
    }

    return bytes;
}

void Server::addClientManager(PlayerManager * playerManager)
{
    playerManager = playerManager;
}

bool Server::addNewConnection(int fd)
{    
    std::cout<<"new connection incoming\n";
    struct sockaddr_in socketParam{};
    socklen_t socketaddrLen = sizeof(socketParam);
    int newFD = accept(fd,(struct sockaddr *)&socketParam,(socklen_t*)&socketaddrLen);
    if(newFD < 0)
    {
        std::cout<<"error on accept socket\n";
        getError("accept");
        return false;
    }

    addFD(newFD);
    
    Player * player = createClient(socketParam);
    playerManager->addPlayer(socketParam.sin_addr.s_addr,player);
    playerManager->addPlayer(newFD,player);
    Message_t message;
    message.text = (char*) Server::WELCOME_MESSAGE;    
    std::cout << message.text;
    std::cout <<message.eventShort;
    Send(player,fd,Server::TCP,(const char*)&message);
    std::cout<<"Message sent"<<std::endl;
    onConnectionCallback();    
}

Player* Server::createClient(sockaddr_in socketParam)
{   
    Player * player = new Player(socketParam.sin_addr.s_addr,socketParam.sin_port);
    return player;
}

void Server::addFD(int fd)
{
    if(activeFds == MAX_FDS_SIZE)
    {
        return;
    }
    fds[activeFds].fd = fd;
    fds[activeFds].events = POLLIN;
    activeFds++;
}



void Server::addListener(Listener listener)
{
    ListenerArray.push_back(listener);

}

void Server::removeListener(int position)
{
    ListenerArray.erase(ListenerArray.begin()+position);
}

void Server::setOnConnection(void(*callback)())
{
    onConnectionCallback = callback;
}

void Server::setOnReceive(void(*callback)(char* buffer))
{
    onReceiveCallback = callback;
}

void Server::setOnDisconnect(void(*callback)())
{
    onDisconnectCallback = callback;
}


void Server::getError(const char* where)
{
    std::cout<< strerror(errno)<<" in : "<< where <<std::endl;
    exit(0);
} 

void Server::purgeFDs()
{
    pollfd newfds[sizeof(unsigned short)];
    for(unsigned short i = 1; i<activeFds; i++)
    {
        if(fcntl(fds[i].fd, F_GETFD) == FD_CLOEXEC)
        {
            continue;
        }
        newfds[i].fd = fds[i].fd;
        newfds[i].events = fds[i].events;
        newfds[i].revents = fds[i].revents;
    }    
    //fds = newfds;
}

void Server::closeAllConnections()
{
    for(unsigned short i = 1; i< activeFds; i++)
    {
        close(fds[i].fd);
    }
}

void Server::Disconnected(int *fd)
{
    *fd = -1;
    close(*fd);    
    onDisconnectCallback();
}
