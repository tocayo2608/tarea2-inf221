#ifndef PATH_RESULT_H
#define PATH_RESULT_H

#include <string>
#include <vector>
struct PathResult {
    bool found;
    long long score;
    std::string path;
    int remaining_lives;
};


PathResult solveBruteForce(int n, int m, int v, int p,
                          const std::vector<std::vector<int>>& types,
                          const std::vector<std::vector<long long>>& values);

PathResult solveDP(int n, int m, int v, int p,
                   const std::vector<std::vector<int>>& types,
                   const std::vector<std::vector<long long>>& values);

PathResult solveGreedy(int n, int m, int v, int p,
                       const std::vector<std::vector<int>>& types,
                       const std::vector<std::vector<long long>>& values);

#endif
