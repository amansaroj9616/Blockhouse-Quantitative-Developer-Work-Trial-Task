CXX = g++
CXXFLAGS = -O3 -std=c++17 -Wall -Wextra

all: reconstruction_aman

reconstruction_aman: main.cpp
	$(CXX) $(CXXFLAGS) -o reconstruction_aman main.cpp

clean:
	rm -f reconstruction_aman mbp_output.csv 