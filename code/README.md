# Documentación

## Entrega

La entrega se realiza vía **aula.usm.cl** en formato `.zip`.

## Multiplicación de matrices

### Programa principal

### Scripts

## Ordenamiento de arreglo unidimensional

Algoritmos: MergeSort, QuickSort, PatienceSort, std::sort.

### Programa principal

### Scripts

## Grid pathfinding

Algoritmos: Fuerza Bruta, Programación Dinámica, Greedy.

### Programa principal

Compile con un compilador C++17 en Linux.

```bash
cd code/grid_pathfinding
make
./grid_pathfinding data/grid_input/4_4_libre_D10_a.txt --greedy
./grid_pathfinding --batch data/grid_input data
```

El modo por lotes lee los archivos `.txt` del directorio de entrada. Escribe
las rutas y puntajes en `data/grid_output/` y las mediciones en
`data/measurements/resultados.csv`. Ejecuta fuerza bruta solo si
`N + M <= 32` y comprueba la coincidencia de puntaje con programación dinámica.
El CSV registra `SIN SOLUCION` cuando un algoritmo no llega a la meta.

Cada fila corresponde a una ejecución en un proceso hijo. El tiempo en
nanosegundos abarca la resolución y reconstrucción del camino. La memoria en
KiB es el pico de memoria residente hasta el fin del cálculo, medido con
`getrusage` en Linux. Incluye la grilla y el costo base del proceso.

### Scripts

`python3 scripts/grid_generator.py --seed 221` genera las 288 instancias con
NumPy. El generador sobrescribe archivos de entrada con el mismo nombre.
