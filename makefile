SRCS = main.cpp MyServer.cpp $(SRCS_LIB)
SRCS_LIB = App.cpp ThreadPool.cpp Server.cpp Supervisor.cpp Client.cpp Log.cpp Console.cpp
OBJ_DIR = ./x64/objects
LIB_DIR = ./x64/lib

LIB_NAME = libserver.a



TARGET = server
CC = g++
CFLAGS = -Wall -g -std=c++17

OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS))
OBJS_LIB = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRCS_LIB))

all: $(TARGET) $(LIB_NAME)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o ./x64/$@ $^

$(OBJ_DIR)/%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_NAME): $(OBJS_LIB)
	ar -rcs $(LIB_DIR)/$@ $^
	cp ./*.hpp ./x64/include/



clean : 
	rm -f $(OBJS) $(TARGET)
	rm -f $(LIB_DIR)/$(LIB_NAME)
	rm -f ./x64/include/*.hpp
	rm -f *.o
	