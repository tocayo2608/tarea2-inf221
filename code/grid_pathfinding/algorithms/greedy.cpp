#include "../path_result.h"

PathResult solveGreedy(int n, int m, int v, int p,
                       const std::vector<std::vector<int>>& types,
                       const std::vector<std::vector<long long>>& values) {
    PathResult resultado{true, values[0][0], "", v};
    int fila = 0;
    int columna = 0;

    while (fila != n - 1 || columna != m - 1) {
        char movimiento = '\0';
        long long mejorGanancia = 0;
        bool mejorEsMina = false;

        for (char candidato : {'R', 'D'}) {
            int siguienteFila = fila + (candidato == 'D');
            int siguienteColumna = columna + (candidato == 'R');
            if (siguienteFila >= n || siguienteColumna >= m) {
                continue;
            }

            int tipo = types[siguienteFila][siguienteColumna];
            bool esMina = tipo == -1;
            if (tipo == 0 || (esMina && resultado.remaining_lives == 1)) {
                continue;
            }

            long long ganancia = esMina ? -static_cast<long long>(p)
                                        : values[siguienteFila][siguienteColumna];
            if (movimiento == '\0' || ganancia > mejorGanancia) {
                movimiento = candidato;
                mejorGanancia = ganancia;
                mejorEsMina = esMina;
            }
        }

        if (movimiento == '\0') {
            return {false, 0, "", 0};
        }

        fila += movimiento == 'D';
        columna += movimiento == 'R';
        resultado.path.push_back(movimiento);
        resultado.score += mejorGanancia;
        resultado.remaining_lives -= mejorEsMina;
    }

    return resultado;
}
