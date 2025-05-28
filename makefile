CXX = g++
CXXFLAGS = -std=c++11
LDFLAGS = -lws2_32
TARGET = server.exe
SRC = server.cpp

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)
