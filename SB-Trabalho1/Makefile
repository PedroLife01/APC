CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2

all: montador ligador

montador: src/montador.cpp src/montador_core.cpp src/montador_core.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/montador.cpp src/montador_core.cpp

ligador: src/ligador.cpp src/montador_core.cpp src/montador_core.hpp
	$(CXX) $(CXXFLAGS) -o $@ src/ligador.cpp src/montador_core.cpp

clean:
	rm -f montador ligador

.PHONY: all clean
