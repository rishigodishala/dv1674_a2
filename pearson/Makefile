CXX = g++
CXXFLAGS = -std=c++17 -O3 -g -Wall -Wextra -ffp-contract=off -pthread
DEPFLAGS = -MMD -MP
CC = gcc
CFLAGS = -O2 -Wall -Wextra

.PHONY: all clean
all: pearson pearson_par verify

pearson: pearson.o vector.o dataset.o analysis.o
	$(CXX) $(CXXFLAGS) $^ -o $@

pearson_par: pearson_par.o vector.o dataset.o analysis.o analysis_par.o
	$(CXX) $(CXXFLAGS) $^ -o $@

verify: verify.c
	$(CC) $(CFLAGS) $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

clean:
	rm -f pearson pearson_par verify *.o *.d

-include $(wildcard *.d)
