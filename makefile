CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -O2 -fsanitize=undefined,address

# Domyślny cel - kompiluje wszystko
all: find_solution

find_solution: find_solution.cpp
	$(CXX) $(CXXFLAGS) find_solution.cpp -o find_solution

clean:
	rm -f *.o find_solution test solve_*