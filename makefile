SRCS = main.cpp App.cpp ThreadPool.cpp MyServer.cpp Server.cpp Supervisor.cpp Client.cpp Log.cpp Console.cpp

OBJ_DIR = ./x64/objects

OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))

TARGET = server

CC = g++
CFLAGS = -Wall -g

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o ./x64/$@

$(OBJ_DIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@


clean : 
	rm -f $(OBJS) $(TARGET)