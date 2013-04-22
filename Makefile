CXX=g++

CFLAGS= -c
CXXFLAGS= -Wall -W -O2 -m64
OUTPUT= client
all: client

client: sockwrap.o client.o
	$(CXX) $(CXXFLAGS) client.o sockwrap.o -o $(OUTPUT)

client.o: client.cpp
	$(CXX) $(CFLAGS) client.cpp 

sockwrap.o: sockwrap.cpp
	$(CXX) $(CFLAGS) sockwrap.cpp sockwrap.h 


clean:
	rm -rf *.o 
distclean:
	rm -rf *.o $(OUTPUT)