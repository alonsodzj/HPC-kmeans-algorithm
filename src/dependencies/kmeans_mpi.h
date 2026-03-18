#pragma once

#include "mpi.h"
#include "omp.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <limits>
#include <ctime>
#include <iomanip>
#include <algorithm>

// Parámetros de configuración del algoritmo
#define MAX_ITERACIONES 2000
#define UMBRAL_CONVERGENCIA 0.0001f  // 5% de puntos cambian

/**
 * @brief Implementación híbrida MPI+OpenMP del algoritmo K-Means
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
    // Inicialización MPI
    int id, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int num_centroides = centroides.size() / n_columnas;
    bool calidad = false;
    int iteraciones = 0;

    if (asignaciones_local.size() != n_filas_local)
        asignaciones_local.resize(n_filas_local, -1);

    //esto es para calcular el tamaño del dataset y ver si se ha llegado a la calidad del umbral especificado.
    //me conmunico con el resto de nodos para saber cuantos tiene y mediante un MPI_Allreduce hago una suma global del número de puntos de cada nodo
    size_t n_filas_total = 0;
    MPI_Allreduce(&n_filas_local, &n_filas_total, 1, MPI_UNSIGNED_LONG_LONG, MPI_SUM, MPI_COMM_WORLD);
    
    // Pre-asignación de buffers (evita malloc/free en el bucle) (mejora de rendimiento)
    std::vector<float> suma_local(centroides.size());
    std::vector<int> conteo_local(num_centroides);
    std::vector<float> suma_global(centroides.size());
    std::vector<int> conteo_global(num_centroides);

    // Bucle principal
    while (iteraciones < MAX_ITERACIONES && !calidad)
    {
        // Sincronizar centroides (solo después de primera iteración)
        if (iteraciones > 0)
        {
            //el nodo 0 hace un broadcast al resto de nodos para actualizar su vector de centroides.
            MPI_Bcast(centroides.data(), centroides.size(), MPI_FLOAT, 0, MPI_COMM_WORLD);
        }

        int desplazados_local = 0;
        std::fill(suma_local.begin(), suma_local.end(), 0.0f);
        std::fill(conteo_local.begin(), conteo_local.end(), 0);

        // Asignación de puntos a centroides (paralelización OpenMP)
        //cuando hay un race condition o hago atomic o critical, en este caso la reducción está tan optimizada que me interesa.
        //se hacen copias locales en cada hilo, se calcula la suma y luego se reduce.

        //esto es para que me soporte la reducción ya que no está hecha para std::Vector
        float* suma_ptr = suma_local.data();
        int* conteo_ptr = conteo_local.data();
        size_t suma_size = suma_local.size();
        int conteo_size = num_centroides;
        /*
        #pragma omp parallel for reduction(+:desplazados_local) \
                                    reduction(+:suma_local.data()[:suma_local.size()]) \
                                    reduction(+:conteo_local.data()[:num_centroides])
        */
        #pragma omp parallel for reduction(+:desplazados_local) \
                         reduction(+:suma_ptr[:suma_size]) \
                         reduction(+:conteo_ptr[:conteo_size])
        //cada nodo procesa sus puntos de forma local
        for (size_t i = 0; i < n_filas_local; ++i)
        {
            //IMPORTANTE -> CADA NODO CONOCE SOLO SUS ESTADÍSTICAS LOCALES, HAY QUE COMUNICARLAS CON EL RESTO DE NODOS
            int mejor_centroide = 0;
            float menor_distancia = std::numeric_limits<float>::max();
            const float* punto = &datos_local[i * n_columnas];

            // Buscar centroide más cercano
            for (int j = 0; j < num_centroides; ++j)
            {
                float dist = 0.0f;
                const float* centro = &centroides[j * n_columnas];

                // Cálculo de distancia euclidiana al cuadrado (vectorizado)
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

            // Tracking de convergencia
            if (asignaciones_local[i] != mejor_centroide)
            {
                desplazados_local++;
                asignaciones_local[i] = mejor_centroide;
            }

            // Acumulación para nuevos centroides (vectorizado)
            conteo_local[mejor_centroide]++;
            float* suma_dest = &suma_local[mejor_centroide * n_columnas];
            #pragma omp simd    //aquí habilito las instrucciones SIMD que me permiten operar más rápido
            //tiene sentido paralelizarlo? dependiendo del número de columnas
            for (size_t d = 0; d < n_columnas; ++d)
            {
                suma_dest[d] += punto[d];
            }
        }

        // Reducción MPI
        int desplazados_total = 0;
        /*  Tengo que comunicar mi suma local, mi conteo, y añadir a una suma global el número de desplazados.
            estos reduce los recibe el nodo 0
            Tengo que comunicar las sumas locales para que luego se puedan calcular los nuevos centroides en base a las sumas locales
            de cada nodo.
        */
        MPI_Reduce(suma_local.data(), suma_global.data(), suma_local.size(), MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);
        MPI_Reduce(conteo_local.data(), conteo_global.data(), num_centroides, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        //este es un AllReduce que me permite hacer la suma acumulada de todos los nodos y comunicarlo con todos.
        MPI_Allreduce(&desplazados_local, &desplazados_total, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

        // Solo el nodo 0 actualiza los centroides, el resto espera
        if (id == 0)
        {
            #pragma omp parallel for
            for (int i = 0; i < num_centroides; ++i)
            {
                if (conteo_global[i] > 0)
                //si ese centroide (grupo) tiene al menos un nodo entre todos
                {
                    float inv_conteo = 1.0f / (float)conteo_global[i];
                    for (size_t j = 0; j < n_columnas; ++j)
                    //actualizo la coordenada del centroide de dicho grupo
                    {
                        //entonces el nuevo centroide tiene en cada coordenada, la suma de todos sus puntos * (1/n_puntos)
                        centroides[i * n_columnas + j] = suma_global[i * n_columnas + j] * inv_conteo;
                    }
                }
            }
            // si además la tasa de cambio de todos se superó entonces se cambia la flag
            float tasa_cambio = static_cast<float>(desplazados_total) / static_cast<float>(n_filas_total);
            if (tasa_cambio <= UMBRAL_CONVERGENCIA) 
                calidad = true;
        }

        // Sincronización de condición de parada
        int calidad_int = calidad ? 1 : 0;
        MPI_Bcast(&calidad_int, 1, MPI_INT, 0, MPI_COMM_WORLD);
        calidad = (calidad_int == 1);
        
        iteraciones++;
    }

    if (id == 0) {
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
        std::cout << "Iteraciones ejecutadas: " << iteraciones << "\n";
        if (iteraciones >= MAX_ITERACIONES) {
            std::cout << "⚠️  ALCANZÓ LÍMITE MÁXIMO (no convergió)\n";
        } else {
            std::cout << "✓ Convergió exitosamente\n";
        }
        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    }
    //paso el vector de centroides actualizado a todos los nodos para que cuando salga al main tenga los datos actualizados.
    //asegura que cuando salgamos de la función, todos los nodos tengan la misma información y puedan operar en base a los mismos resultados
    MPI_Bcast(centroides.data(), centroides.size(), MPI_FLOAT, 0, MPI_COMM_WORLD);
}