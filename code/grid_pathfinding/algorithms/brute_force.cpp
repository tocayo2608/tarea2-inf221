#include "../path_result.h"

static void explorar(int fila, int columna, int vidas, long long puntaje,
                     int n, int m, int penalizacion,
                     const std::vector<std::vector<int>>& tipos,
                     const std::vector<std::vector<long long>>& valores,
                     std::string& camino, PathResult& mejor) {
    int tipo = tipos[fila][columna];
    if (tipo == 0) {
        return;
    }

    if (tipo == -1) {
        vidas--;
        if (vidas == 0) {
            return;
        }
        puntaje -= penalizacion;
    } else {
        puntaje += valores[fila][columna];
    }

    if (fila == n - 1 && columna == m - 1) {
        if (!mejor.found || puntaje > mejor.score) {
            mejor = {true, puntaje, camino, vidas};
        }
        return;
    }

    if (columna + 1 < m) {
        camino.push_back('R');
        explorar(fila, columna + 1, vidas, puntaje, n, m, penalizacion,
                 tipos, valores, camino, mejor);
        camino.pop_back();
    }

    if (fila + 1 < n) {
        camino.push_back('D');
        explorar(fila + 1, columna, vidas, puntaje, n, m, penalizacion,
                 tipos, valores, camino, mejor);
        camino.pop_back();
    }
}

PathResult solveBruteForce(int n, int m, int v, int p,
                          const std::vector<std::vector<int>>& types,
                          const std::vector<std::vector<long long>>& values) {
    PathResult mejor{false, 0, "", 0};
    std::string camino;
    explorar(0, 0, v, 0, n, m, p, types, values, camino, mejor);
    return mejor;
}
