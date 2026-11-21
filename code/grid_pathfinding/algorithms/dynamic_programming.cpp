#include "../path_result.h"
#include <algorithm>

struct EstadoDP {
    long long puntaje = 0;
    char movimiento = '\0';
};

PathResult solveDP(int n, int m, int v, int p,
                   const std::vector<std::vector<int>>& types,
                   const std::vector<std::vector<long long>>& values) {

    std::vector<std::vector<EstadoDP>> tabla(n, std::vector<EstadoDP>(m * v));

    for (int fila = 0; fila < n; fila++) {
        for (int columna = 0; columna < m; columna++) {
            int tipo = types[fila][columna];
            if (tipo == 0) {
                continue;
            }

            int perdida = (tipo == -1) ? 1 : 0;
            long long ganancia = (tipo == -1) ? -static_cast<long long>(p)
                                             : values[fila][columna];

            if (fila == 0 && columna == 0 && v > perdida) {
                tabla[0][v - perdida - 1] = {ganancia, 'S'};
            }

            for (int vidas = 1; vidas <= v - perdida; vidas++) {
                int indice = columna * v + vidas - 1;
                EstadoDP& actual = tabla[fila][indice];

                if (columna > 0) {
                    const EstadoDP& izquierda = tabla[fila][indice - v + perdida];
                    if (izquierda.movimiento != '\0') {
                        actual = {izquierda.puntaje + ganancia, 'R'};
                    }
                }

                if (fila > 0) {
                    const EstadoDP& arriba = tabla[fila - 1][indice + perdida];
                    if (arriba.movimiento != '\0' &&
                        (actual.movimiento == '\0' || arriba.puntaje + ganancia > actual.puntaje)) {
                        actual = {arriba.puntaje + ganancia, 'D'};
                    }
                }
            }
        }
    }

    PathResult resultado{false, 0, "", 0};
    for (int vidas = 1; vidas <= v; vidas++) {
        const EstadoDP& destino = tabla[n - 1][(m - 1) * v + vidas - 1];
        if (destino.movimiento != '\0' &&
            (!resultado.found || destino.puntaje > resultado.score)) {
            resultado = {true, destino.puntaje, "", vidas};
        }
    }

    if (!resultado.found) {
        return resultado;
    }

    int fila = n - 1;
    int columna = m - 1;
    int vidas = resultado.remaining_lives;
    while (fila > 0 || columna > 0) {
        char movimiento = tabla[fila][columna * v + vidas - 1].movimiento;
        resultado.path.push_back(movimiento);
        if (types[fila][columna] == -1) {
            vidas++;
        }

        if (movimiento == 'R') {
            columna--;
        } else {
            fila--;
        }
    }
    std::reverse(resultado.path.begin(), resultado.path.end());
    return resultado;
}
