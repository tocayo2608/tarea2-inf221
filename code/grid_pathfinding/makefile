CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

FUENTES = grid_pathfinding.cpp algorithms/brute_force.cpp algorithms/dynamic_programming.cpp algorithms/greedy.cpp

grid_pathfinding: $(FUENTES) path_result.h
	$(CXX) $(CXXFLAGS) $(FUENTES) -o $@

.PHONY: clean
clean:
	$(RM) grid_pathfinding
