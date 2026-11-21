// INF-221, Tarea 2, 2026-2.
// Referencia: apéndice A del enunciado para la lectura y el formato del resultado.

#include <fstream>
#include <iostream>
#include "path_result.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " archivo_de_grilla\n";
        return 1;
    }

    std::ifstream entrada(argv[1]);
    if (!entrada) {
        std::cerr << "No se pudo abrir " << argv[1] << '\n';
        return 1;
    }

    int n, m, vidas, penalizacion;
    if (!(entrada >> n >> m >> vidas >> penalizacion) ||
        n <= 0 || m <= 0 || vidas <= 0) {
        std::cerr << "Encabezado de grilla inválido.\n";
        return 1;
    }

    std::vector<std::vector<int>> tipos(n, std::vector<int>(m));
    std::vector<std::vector<long long>> valores(n, std::vector<long long>(m));
    for (int fila = 0; fila < n; fila++) {
        for (int columna = 0; columna < m; columna++) {
            if (!(entrada >> tipos[fila][columna]) ||
                tipos[fila][columna] < -1 || tipos[fila][columna] > 1) {
                std::cerr << "Matriz de tipos inválida.\n";
                return 1;
            }
        }
    }

    for (int fila = 0; fila < n; fila++) {
        for (int columna = 0; columna < m; columna++) {
            if (!(entrada >> valores[fila][columna])) {
                std::cerr << "Matriz de valores inválida.\n";
                return 1;
            }
        }
    }

    PathResult resultado = solveDP(n, m, vidas, penalizacion, tipos, valores);
    if (static_cast<long long>(n) + m <= 32) {
        PathResult fuerzaBruta = solveBruteForce(n, m, vidas, penalizacion, tipos, valores);
        if (resultado.found != fuerzaBruta.found ||
            (resultado.found && resultado.score != fuerzaBruta.score)) {
            std::cerr << "Fuerza bruta y programación dinámica difieren en "
                      << argv[1] << '\n';
            return 1;
        }
    }
    if (resultado.found) {
        std::cout << resultado.score << '\n'
                  << resultado.path << '\n'
                  << resultado.remaining_lives << '\n';
    } else {
        std::cout << "SIN SOLUCION\n";
    }
    return 0;
}
