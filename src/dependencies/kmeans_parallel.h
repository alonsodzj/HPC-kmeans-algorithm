#pragma once

#include "mpi.h"
#include "omp.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <limits>
#include <ctime>
#include <iomanip>

/**
 * @param datos_local Vector plano con los datos del dataset.
 * @param n_filas_local Número de puntos locales.
 * @param n_columnas Número de dimensiones de cada punto.
 * @param centroides Vector con los centroides iniciales (se actualizan in-place).
 * @param asignaciones_local Vector donde se guarda el clúster de cada punto.
 */
void kmeans_mpi(const std::vector<float>& datos_local, size_t n_filas_local, size_t n_columnas, std::vector<float>& centroides, std::vector<int>& asignaciones_local)
{
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int num_centroides = centroides.size() / n_columnas;

    bool calidad = false;
    int iteraciones = 0;

    // asegurar tamaño del vector de asignaciones
    if (asignaciones_local.size() != n_filas_local)
        asignaciones_local.resize(n_filas_local, -1);

    // calcular número total de filas
    size_t n_filas_total = 0;

    // hace la suma de todos los nodos y lo envía a todos los nodos
    MPI_Allreduce(&n_filas_local, &n_filas_total, 1, MPI_UNSIGNED_LONG, MPI_SUM, MPI_COMM_WORLD);

    while (iteraciones < 2000 && !calidad)
    {
        // Sincronizo los centroides con un broadcast al resto de nodos
        MPI_Bcast(centroides.data(), centroides.size(), MPI_FLOAT, 0, MPI_COMM_WORLD);

        int desplazados_local = 0;  // esto me servirá para sumarlo con los del resto de nodos y ver si superamos el umbral

        std::vector<float> suma_local(centroides.size(), 0.0f); // almacenar la suma acumulada de los datos de cada coordenada antes de hacer la media
        std::vector<int> conteo_local(num_centroides, 0);   // lleva la cuenta del número de puntos que tiene cada grupo

        // Reducción paralela optimizada: OpenMP gestiona las copias privadas y la suma final de forma eficiente
        #pragma omp parallel for reduction(+:desplazados_local, \
                                              suma_local[:suma_local.size()], \
                                              conteo_local[:num_centroides])
        for (size_t i = 0; i < n_filas_local; ++i)
        // por cada punto
        {
            int mejor_centroide = 0;
            float menor_distancia = std::numeric_limits<float>::max(); // con esto no hay cálculo en tiempo de ejecución el compilador lo pone directamente.

            for (int j = 0; j < num_centroides; ++j)
            // por cada centroide calculo la distancia a este y veo cual es el que más me conviene
            {
                float dist = 0.0f;
                for (size_t d = 0; d < n_columnas; ++d)
                {
                    float diff = datos_local[i * n_columnas + d] - centroides[j * n_columnas + d];
                    dist += diff * diff;
                }

                if (dist < menor_distancia) // si el centroide mejora la distancia entonces lo cambio.
                {
                    menor_distancia = dist;
                    mejor_centroide = j;
                }
            }

            if (asignaciones_local[i] != mejor_centroide)
            {
                desplazados_local++; 
                asignaciones_local[i] = mejor_centroide;
            }

            conteo_local[mejor_centroide]++;

            for (size_t d = 0; d < n_columnas; ++d)
            {
                suma_local[mejor_centroide * n_columnas + d] += datos_local[i * n_columnas + d];
            }
        } // Fin de sección paralela optimizada

        // C. reducción MPI fuera de la zona paralela
        std::vector<float> suma_global(centroides.size());
        std::vector<int> conteo_global(num_centroides);

        int desplazados_total = 0;

        MPI_Reduce(suma_local.data(), suma_global.data(), suma_local.size(), MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(conteo_local.data(), conteo_global.data(), num_centroides, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

        MPI_Allreduce(&desplazados_local, &desplazados_total, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

        // D. actualizar centroides
        if (rank == 0)  // si soy el nodo 0
        {
            #pragma omp parallel for    // creo los hilos y ejecuto secuencialmente, paralelizo el de fuera
            for (int i = 0; i < num_centroides; ++i)
            // por cada centroide
            {
                if (conteo_global[i] > 0)
                // si tiene más de un punto asignado
                {
                    for (size_t j = 0; j < n_columnas; ++j)
                    // por cada columna
                    {
                        centroides[i * n_columnas + j] = suma_global[i * n_columnas + j] / (float)conteo_global[i];
                    }
                }
            }
            float tasa_cambio = static_cast<float>(desplazados_total) / (float)n_filas_total;
            if (tasa_cambio <= 0.001f) calidad = true;
        }
        // comunicar parada al resto de nodos
        MPI_Bcast(&calidad, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
        iteraciones++;
    }
}