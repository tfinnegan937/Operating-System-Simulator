CC = g++

CFlags =  -fpthread

SOURCES = ./main.cpp ./config_parser.cpp ./Config.cpp ./input_parser.cpp ./Simulator.cpp ./PCB.cpp ./Semaphore.cpp ./Process.cpp
OBJECTS = $(SOURCES:.cpp=.o)
TARGET = assignment5

$(TARGET):
	$(CC) -g -O0 -std=c++14 -pthread -o $@ $(SOURCES)


    
.PHONY: clean

clean:
	@rm -f $(TARGET) $(OBJECTS) *.lgf core
