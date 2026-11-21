// INF-221, Tarea 2, 2026-2.
// Referencia: apéndice A y sección «Mediciones de tiempo, memoria y calidad».
// Cada medición usa un proceso hijo para obtener su pico de RSS por separado.

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#include "path_result.h"

namespace fs = std::filesystem;

struct Grilla {
    int n, m, vidas, penalizacion;
    std::vector<std::vector<int>> tipos;
    std::vector<std::vector<long long>> valores;
};

static bool leerGrilla(const fs::path& ruta, Grilla& grilla) {
    std::ifstream entrada(ruta);
    if (!entrada) {
        std::cerr << "No se pudo abrir " << ruta << '\n';
        return false;
    }

    auto& [n, m, vidas, penalizacion, tipos, valores] = grilla;
    if (!(entrada >> n >> m >> vidas >> penalizacion) ||
        n <= 0 || m <= 0 || vidas <= 0) {
        std::cerr << "Encabezado de grilla inválido.\n";
        return false;
    }

    tipos.assign(n, std::vector<int>(m));
    valores.assign(n, std::vector<long long>(m));
    for (int fila = 0; fila < n; fila++) {
        for (int columna = 0; columna < m; columna++) {
            if (!(entrada >> tipos[fila][columna]) ||
                tipos[fila][columna] < -1 || tipos[fila][columna] > 1) {
                std::cerr << "Matriz de tipos inválida.\n";
                return false;
            }
        }
    }

    for (int fila = 0; fila < n; fila++) {
        for (int columna = 0; columna < m; columna++) {
            if (!(entrada >> valores[fila][columna])) {
                std::cerr << "Matriz de valores inválida.\n";
                return false;
            }
        }
    }

    return true;
}

static void escribirResultado(std::ostream& salida, const PathResult& resultado) {
    if (resultado.found) {
        salida << resultado.score << '\n'
               << resultado.path << '\n'
               << resultado.remaining_lives << '\n';
    } else {
        salida << "SIN SOLUCION\n";
    }
}

using Solver = decltype(&solveDP);

static PathResult resolver(Solver solver, const Grilla& grilla) {
    const auto& [n, m, vidas, penalizacion, tipos, valores] = grilla;
    return solver(n, m, vidas, penalizacion, tipos, valores);
}

static bool aplicaFuerzaBruta(const Grilla& grilla) {
    return static_cast<long long>(grilla.n) + grilla.m <= 32;
}

struct Medicion {
    long long tiempoNs;
    long long puntaje;
    long memoriaKiB;
    bool encontrada;
};

static bool medir(Solver solver, const Grilla& grilla, const fs::path& ruta,
                  Medicion& medicion) {
    int canal[2];
    if (pipe(canal) == -1) {
        return false;
    }

    pid_t hijo = fork();
    if (hijo == 0) {
        close(canal[0]);
        auto inicio = std::chrono::steady_clock::now();
        PathResult resultado = resolver(solver, grilla);
        auto duracion = std::chrono::steady_clock::now() - inicio;
        struct rusage uso{};
        if (getrusage(RUSAGE_SELF, &uso) == -1) {
            _exit(1);
        }
        Medicion datos{
            std::chrono::duration_cast<std::chrono::nanoseconds>(duracion).count(),
            resultado.score, uso.ru_maxrss, resultado.found
        };

        std::ofstream salida(ruta);
        escribirResultado(salida, resultado);
        salida.close();
        if (!salida) {
            _exit(1);
        }

        ssize_t escritos = write(canal[1], &datos, sizeof(datos));
        _exit(escritos == sizeof(datos) ? 0 : 1);
    }

    close(canal[1]);
    if (hijo == -1) {
        close(canal[0]);
        return false;
    }

    ssize_t leidos;
    do {
        leidos = read(canal[0], &medicion, sizeof(medicion));
    } while (leidos == -1 && errno == EINTR);
    close(canal[0]);

    int estado;
    pid_t terminado;
    do {
        terminado = waitpid(hijo, &estado, 0);
    } while (terminado == -1 && errno == EINTR);
    if (terminado != hijo || !WIFEXITED(estado) || WEXITSTATUS(estado) != 0 ||
        leidos != sizeof(medicion)) {
        return false;
    }

    return true;
}

static bool coinciden(bool encontrada, long long puntaje,
                      bool otraEncontrada, long long otroPuntaje) {
    return encontrada == otraEncontrada && (!encontrada || puntaje == otroPuntaje);
}

static int ejecutarLote(const fs::path& entradas, const fs::path& datos) {
    std::vector<fs::path> instancias;
    for (const auto& archivo : fs::directory_iterator(entradas)) {
        if (archivo.is_regular_file() && archivo.path().extension() == ".txt") {
            instancias.push_back(archivo.path());
        }
    }
    if (instancias.empty()) {
        std::cerr << "No hay archivos de grilla en " << entradas << '\n';
        return 1;
    }
    std::sort(instancias.begin(), instancias.end());

    fs::path salidas = datos / "grid_output";
    fs::path mediciones = datos / "measurements";
    fs::create_directories(salidas);
    fs::create_directories(mediciones);
    fs::path definitivo = mediciones / "resultados.csv";
    fs::path temporal = mediciones / "resultados.csv.tmp";
    std::ofstream csv(temporal);
    if (!csv) {
        std::cerr << "No se pudo abrir " << temporal << '\n';
        return 1;
    }
    csv << "instancia,n,m,algoritmo,tiempo_ns,memoria_kib,puntaje\n";

    for (const auto& instancia : instancias) {
        Grilla grilla;
        if (!leerGrilla(instancia, grilla)) {
            return 1;
        }

        std::string nombre = instancia.stem().string();
        const std::pair<const char*, Solver> algoritmos[] = {
            {"dp", solveDP}, {"greedy", solveGreedy}, {"bruteforce", solveBruteForce}
        };
        Medicion optimo{};
        for (const auto& [algoritmo, solver] : algoritmos) {
            if (solver == solveBruteForce && !aplicaFuerzaBruta(grilla)) {
                continue;
            }

            fs::path ruta = salidas / (nombre + "_" + algoritmo + "_out.txt");
            Medicion medicion{};
            if (!medir(solver, grilla, ruta, medicion)) {
                std::cerr << "Falló la medición de " << algoritmo << " en " << instancia << '\n';
                return 1;
            }

            if (solver == solveDP) {
                optimo = medicion;
            }
            if (solver == solveBruteForce &&
                !coinciden(optimo.encontrada, optimo.puntaje, medicion.encontrada, medicion.puntaje)) {
                std::cerr << "Fuerza bruta y programación dinámica difieren en " << instancia << '\n';
                return 1;
            }

            csv << nombre << ',' << grilla.n << ',' << grilla.m << ',' << algoritmo << ','
                << medicion.tiempoNs << ',' << medicion.memoriaKiB << ',';
            if (medicion.encontrada) {
                csv << medicion.puntaje;
            } else {
                csv << "SIN SOLUCION";
            }
            csv << '\n';
        }
    }

    csv.close();
    if (!csv) {
        std::cerr << "No se pudieron guardar las mediciones en " << temporal << '\n';
        return 1;
    }
    fs::rename(temporal, definitivo);
    std::cout << "Instancias procesadas: " << instancias.size() << '\n';
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc == 4 && std::string(argv[1]) == "--batch") {
        try {
            return ejecutarLote(argv[2], argv[3]);
        } catch (const fs::filesystem_error& error) {
            std::cerr << error.what() << '\n';
            return 1;
        }
    }
    if (argc != 2 && !(argc == 3 && std::string(argv[2]) == "--greedy")) {
        std::cerr << "Uso: " << argv[0] << " archivo_de_grilla [--greedy]\n"
                  << "     " << argv[0] << " --batch directorio_entradas directorio_datos\n";
        return 1;
    }

    Grilla grilla;
    if (!leerGrilla(argv[1], grilla)) {
        return 1;
    }
    bool ejecutarGreedy = argc == 3;
    PathResult resultado = resolver(ejecutarGreedy ? solveGreedy : solveDP, grilla);
    if (!ejecutarGreedy && aplicaFuerzaBruta(grilla)) {
        PathResult fuerzaBruta = resolver(solveBruteForce, grilla);
        if (!coinciden(resultado.found, resultado.score, fuerzaBruta.found, fuerzaBruta.score)) {
            std::cerr << "Fuerza bruta y programación dinámica difieren en " << argv[1] << '\n';
            return 1;
        }
    }
    escribirResultado(std::cout, resultado);
    return 0;
}
