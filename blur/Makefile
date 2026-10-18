CXX = g++
CXXFLAGS = -std=c++17 -O3 -g -Wall -Wextra -ffp-contract=off -pthread
DEPFLAGS = -MMD -MP
CC = gcc
CFLAGS = -O2 -Wall -Wextra

.PHONY: all clean
all: blur blur_par

blur: blur.o matrix.o ppm.o filters.o
	$(CXX) $(CXXFLAGS) $^ -o $@

blur_par: blur_par.o matrix.o ppm.o filters.o filters_par.o
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

clean:
	rm -f blur blur_par *.o *.d

-include $(wildcard *.d)
