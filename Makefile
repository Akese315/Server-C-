

server.out : main.o Server.o PlayerManager.o Player.o Client.o EventData.o
	g++ -o server.out main.o Server.o PlayerManager.o Player.o Client.o EventData.o

Server.o : Server.cpp Server.h
	g++ -g -o Server.o -c Server.cpp

PlayerManager.o : PlayerManager.cpp PlayerManager.h
	g++ -g -o PlayerManager.o -c PlayerManager.cpp

Player.o : Player.cpp Player.h
	g++ -g -o Player.o -c Player.cpp

Client.o : Client.cpp Client.h
	g++ -g -o Client.o -c Client.cpp

EventData.o : EventData.cpp EventData.h
	g++ -g -o EventData.o -c EventData.cpp
	

clean : 
	rm *.o
