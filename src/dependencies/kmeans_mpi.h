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
 * @brief Implementación SoA del algoritmo k-means, tengo un vector de columnas.
 * 
 * @param datos_local Vector plano con los datos del dataset
 * @param n_filas_local Número de puntos locales en este proceso
 * @param n_columnas Número de dimensiones de cada punto
 * @param centroides Vector con los centroides iniciales (se actualizan in-place) ASUME QUE SE LES PASA EXTERNAMENTE
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

    int num_centroides = centroides.size() / n_columnas;
    bool calidad = false;
    int iteraciones = 0;

    if (asignaciones_local.size() != n_filas_local)
    {
        asignaciones_local.resize(n_filas_local, -1);
    }
    
    size_t n_filas_total = 0;
    MPI_Allreduce(&n_filas_local, &n_filas_total, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);
    
    std::vector<float> suma_local(centroides.size());
    std::vector<int> conteo_local(num_centroides);
    std::vector<float> suma_global(centroides.size());
    std::vector<int> conteo_global(num_centroides);

    while (iteraciones < MAX_ITERACIONES && !calidad)
    {
        if (iteraciones > 0)
        {
            MPI_Bcast(centroides.data(), centroides.size(), MPI_FLOAT, 0, MPI_COMM_WORLD);
        }

        int desplazados_local = 0;

        //relleno ambos vectores con 0
        std::fill(suma_local.begin(), suma_local.end(), 0.0f);  
        std::fill(conteo_local.begin(), conteo_local.end(), 0);

        //creo punteros para que se soporte la reducción.
        float* suma_ptr = suma_local.data();
        int* conteo_ptr = conteo_local.data();
        size_t suma_size = suma_local.size();
        int conteo_size = num_centroides;

        #pragma omp parallel for reduction(+:desplazados_local) \
                         reduction(+:suma_ptr[:suma_size]) \
                         reduction(+:conteo_ptr[:conteo_size])

        for (size_t i = 0; i < n_filas_local; ++i)
        //por cada punto
        {
            int mejor_centroide = 0;
            float menor_distancia = std::numeric_limits<float>::max();
            const float* punto = &datos_local[i * n_columnas];  //puntero a donde empieza el punto

            for (int j = 0; j < num_centroides; ++j)
            //por cada centroide calclulo la distancia a este y busco el más cercano
            {
                float dist = 0.0f;
                const float* centro = &centroides[j * n_columnas];  //referencia al centroide
                
                #pragma omp simd reduction(+:dist)
                for (size_t d = 0; d < n_columnas; ++d)
                //por cada columna tanto del punto como del centroide
                {
                    float diff = punto[d] - centro[d];
                    dist += diff * diff;    //calculo el acumulado dentro de la variable dist
                }
                if (dist < menor_distancia) //si la distancia es menor a dicho centroide
                {
                    menor_distancia = dist; //actualizo la distancia.
                    mejor_centroide = j;    //guardo la referencia al centroide 
                }
            }
            //actualizo el centroide si no es el actual y sumo desplazados
            if (asignaciones_local[i] != mejor_centroide)
            {
                desplazados_local++;
                asignaciones_local[i] = mejor_centroide;
            }

            //actualizo el número de puntos que pertenecen al centroide
            conteo_local[mejor_centroide]++;
            float* suma_dest = &suma_local[mejor_centroide * n_columnas];   //referencia a la suma de mi mejor centroide
            //habilito las instrucciones SIMD
            #pragma omp simd
            for (size_t d = 0; d < n_columnas; ++d)
            //por cada columna
            {
                suma_dest[d] += punto[d];   //le sumo el valor de dicha coordenada para ese punto para luego calcular la media 
            }
        }

        //REDUCCIÓN MPI
        int desplazados_total = 0;
        MPI_Reduce(suma_local.data(), suma_global.data(), suma_local.size(), MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(conteo_local.data(), conteo_global.data(), num_centroides, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Allreduce(&desplazados_local, &desplazados_total, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

        // Solo el nodo 0 actualiza los centroides, el resto espera
        if (id == 0)
        {
            //paralelizo el bucle exterior
            #pragma omp parallel for
            for (int i = 0; i < num_centroides; ++i)
            //por cada centroide tengo que hacer una división (calcular la media, ya he sumado todos los valores)
            {
                if (conteo_global[i] > 0)
                //si tiene al menos un punto
                {
                    //solo hago una división (más costosa que la multiplicación) en lugar de hacer n_columnas divisiones.
                    float inv_conteo = 1.0f / (float)conteo_global[i];
                    for (size_t j = 0; j < n_columnas; ++j)
                    //por cada coordenada del centroide
                    {
                        //actualizo con una multiplicación
                        centroides[i * n_columnas + j] = suma_global[i * n_columnas + j] * inv_conteo;
                    }
                }
            }
            // si además la tasa de cambio de todos se superó entonces se cambia la flag
            float tasa_cambio = static_cast<float>(desplazados_total) / static_cast<float>(n_filas_total);
            if (tasa_cambio <= UMBRAL_CONVERGENCIA) calidad = true;
        }

        // Sincronización de condición de parada (lo hago mejor con MPI_INT)
        int calidad_int = calidad ? 1 : 0;
        MPI_Bcast(&calidad_int, 1, MPI_INT, 0, MPI_COMM_WORLD);
        calidad = (calidad_int == 1);
        
        iteraciones++;
    }

    //esto es para la depuración
    if (id == 0) {
        std::cout << "Iteraciones ejecutadas: " << iteraciones << "\n";
        if (iteraciones >= MAX_ITERACIONES) {
            std::cout << "ALCANZÓ LÍMITE MÁXIMO (no convergió)\n";
        } else {
            std::cout << "Convergió exitosamente\n";
        }
    }
    //paso el vector de centroides actualizado a todos los nodos para que cuando salga al main tenga los datos actualizados.
    //asegura que cuando salgamos de la función, todos los nodos tengan la misma información y puedan operar en base a los mismos resultados
    MPI_Bcast(centroides.data(), centroides.size(), MPI_FLOAT, 0, MPI_COMM_WORLD);
}