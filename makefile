CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

all: fcm generator

fcm: fcm.cpp
	$(CXX) $(CXXFLAGS) -o fcm fcm.cpp

generator: generator.cpp
	$(CXX) $(CXXFLAGS) -o generator generator.cpp

clean:
	rm -f fcm generator
