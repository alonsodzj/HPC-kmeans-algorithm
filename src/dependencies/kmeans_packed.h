#pragma once

#include "mpi.h"
#include "omp.h"
#include "constantes.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <limits>
#include <ctime>
#include <iomanip>
#include <algorithm>

/**
 * @brief Implementación SoA del algoritmo k-means con fusión de MPI_Reduce.
 * @param datos_local       Vector plano con los datos del dataset (SoA)
 * @param n_filas_local     Número de puntos locales en este proceso
 * @param n_columnas        Número de dimensiones de cada punto
 * @param centroides        Centroides iniciales (actualizados in-place)
 * @param asignaciones_local Vector donde se guarda el clúster de cada punto
 */
void kmeans_mpi(const std::vector<float>& datos_local,
                size_t n_filas_local,
                size_t n_columnas,
                std::vector<float>& centroides,
                std::vector<int>& asignaciones_local)
{
    int id, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int    num_centroides = static_cast<int>(centroides.size() / n_columnas);
    const size_t stride         = n_columnas + 1;  // D floats de suma + 1 float de conteo

    bool calidad    = false;
    int  iteraciones = 0;

    if (asignaciones_local.size() != n_filas_local)
        asignaciones_local.resize(n_filas_local, -1);

    // Calcula el total global de filas una sola vez
    size_t n_filas_total = 0;
    MPI_Allreduce(&n_filas_local, &n_filas_total, 1,
                  MPI_UNSIGNED_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);

    //empaqueto el buffer para madnar el núm
    const size_t packed_size = static_cast<size_t>(num_centroides) * stride;
    std::vector<float> packed_local (packed_size);
    std::vector<float> packed_global(packed_size);

    while (iteraciones < MAX_ITERACIONES && !calidad)
    {
        // A partir de la 2ª iteración los centroides ya están sincronizados
        // desde el Bcast al final del bucle anterior; en la 1ª no hace falta.
        if (iteraciones > 0)
            MPI_Bcast(centroides.data(), centroides.size(), MPI_FLOAT, 0, MPI_COMM_WORLD);

        int desplazados_local = 0;

        std::fill(packed_local.begin(), packed_local.end(), 0.0f);

        // Punteros raw para que OpenMP acepte la reducción de arrays
        float* packed_ptr  = packed_local.data();
        size_t packed_sz   = packed_size;

        #pragma omp parallel for reduction(+:desplazados_local) \
                                 reduction(+:packed_ptr[:packed_sz])
        for (size_t i = 0; i < n_filas_local; ++i)
        {
            int   mejor_centroide = 0;
            float menor_distancia = std::numeric_limits<float>::max();
            const float* punto    = &datos_local[i * n_columnas];

            for (int j = 0; j < num_centroides; ++j)
            {
                float        dist   = 0.0f;
                const float* centro = &centroides[j * n_columnas];

                #pragma omp simd reduction(+:dist)
                for (size_t d = 0; d < n_columnas; ++d)
                {
                    float diff = punto[d] - centro[d];
                    dist += diff * diff;
                }

                if (dist < menor_distancia)
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

            // Acumular en buffer empaquetado:
            //   posiciones [0 .. D-1]  → suma de coordenadas
            //   posición   [D]         → conteo (como float)
            float* dest = &packed_local[mejor_centroide * stride];

            #pragma omp simd
            for (size_t d = 0; d < n_columnas; ++d)
                dest[d] += punto[d];

            dest[n_columnas] += 1.0f;
        }

        // Una sola llamada en lugar de dos: ahorramos una barrera de red
        // completa por iteración.
        int desplazados_total = 0;
        MPI_Reduce(packed_local.data(), packed_global.data(), static_cast<int>(packed_size), MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Allreduce(&desplazados_local, &desplazados_total, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

        if (id == 0)
        {
            #pragma omp parallel for
            for (int i = 0; i < num_centroides; ++i)
            {
                const float* src    = &packed_global[i * stride];
                float conteo = src[n_columnas];   // último elemento = conteo

                if (conteo > 0.0f)
                {
                    // Multiplicación en lugar de división (igual que antes)
                    float inv_conteo = 1.0f / conteo;

                    #pragma omp simd
                    for (size_t j = 0; j < n_columnas; ++j)
                        centroides[i * n_columnas + j] = src[j] * inv_conteo;
                }
            }

            float tasa_cambio = static_cast<float>(desplazados_total)
                              / static_cast<float>(n_filas_total);
            if (tasa_cambio <= UMBRAL_CONVERGENCIA)
                calidad = true;
        }

        // Sincronizar condición de parada y centroides en un solo Bcast
        int calidad_int = calidad ? 1 : 0;
        MPI_Bcast(&calidad_int, 1, MPI_INT, 0, MPI_COMM_WORLD);
        calidad = (calidad_int == 1);

        iteraciones++;
    }

    if (id == 0)
    {
        std::cout << "Iteraciones ejecutadas: " << iteraciones << "\n";
        if (iteraciones >= MAX_ITERACIONES)
            std::cout << "ALCANZÓ LÍMITE MÁXIMO (no convergió)\n";
        else
            std::cout << "Convergió exitosamente\n";
    }

    // Asegura que todos los nodos tengan los centroides finales actualizados
    MPI_Bcast(centroides.data(), centroides.size(), MPI_FLOAT, 0, MPI_COMM_WORLD);
}