#pragma once

#include "mpi.h"
#include "omp.h"
#include <vector>
#include <iostream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <algorithm>

/*
cuando ya he terminado de ejecutar mi algoritmo kmedias, lo que hago es que cada nodo calcula las estadísticas de sus puntos
en base a los grupos resultantes.
El nodo 0 calcula las estadísticas para grupo en base a cada uno de los puntos que tiene asignado,
el 1 hace lo mismo, el 2 igual....

El resultado global y las esatdísticas correctas serán la combinación de estos cálculos locales en cada nodo

*/

//creo un struct que me vendrá bien a la hora de exportar las estadísticas al main
struct Estadisticas{
    float min       = std::numeric_limits<float>::max();        //para no inicializar 0
    float max       = std::numeric_limits<float>::lowest();     //para no inicializar a 0
    float media     = 0;
    float varianza  = 0;
    int contador    = 0;
}__attribute__((packed));

/**
 * @brief Calcula min, max, media y varianza en UNA SOLA PASADA
 * @param datos_local           el vector con los datos
 * @param n_filas_local         el número de puntos
 * @param n_columnas            el número de coordenadas
 * @param centroides            el vector con los centroides
 * @param asignaciones_local    el vector con los centroides
 * @param num_cluesters         el número de clústeres
 */
void calcularEstadisticasMPI(
    const std::vector<float>& datos_local,
    size_t n_filas_local,
    size_t n_columnas,
    const std::vector<float>& centroides,
    const std::vector<int>& asignaciones_local, 
    int num_clusters)
{
    int id, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    size_t total_dims = num_clusters * n_columnas;
    
    //puedo tener un struct para esto pero dudo que lo haga más rapido
    std::vector<float> min_local(total_dims, std::numeric_limits<float>::max());
    std::vector<float> max_local(total_dims, std::numeric_limits<float>::lowest());
    std::vector<double> suma_cuadrados_local(total_dims, 0.0);
    std::vector<int> conteo_local(num_clusters, 0);
    
    // Paralelización OpenMP con copias privadas
    #pragma omp parallel
    {
        //creo una copia local de cada una de las estructuras
        std::vector<float> min_private(total_dims, std::numeric_limits<float>::max());
        std::vector<float> max_private(total_dims, std::numeric_limits<float>::lowest());
        std::vector<double> suma2_private(total_dims, 0.0);
        std::vector<int> conteo_private(num_clusters, 0);
        
        #pragma omp for nowait schedule(static) //static es caché friendly
        for (size_t i = 0; i < n_filas_local; ++i) {
            int cluster = asignaciones_local[i];
            const float* punto = &datos_local[i * n_columnas];
            
            conteo_private[cluster]++;
            
            size_t base_idx = cluster * n_columnas;
            
            for (size_t d = 0; d < n_columnas; ++d) {
                float valor = punto[d];
                size_t idx = base_idx + d;
                
                min_private[idx] = std::min(min_private[idx], valor);
                max_private[idx] = std::max(max_private[idx], valor);
                suma2_private[idx] += static_cast<double>(valor) * valor;
            }
        }
        

        //aquí hay un cuello de botella que flipas loco
        //combinar los resultados, crítical no es óptimo tengo que buscar la manera de paralelizarlo
        #pragma omp critical
        {
            for (int c = 0; c < num_clusters; ++c) {
                conteo_local[c] += conteo_private[c];
                
                size_t base_idx = c * n_columnas;
                for (size_t d = 0; d < n_columnas; ++d) {
                    size_t idx = base_idx + d;
                    min_local[idx] = std::min(min_local[idx], min_private[idx]);
                    max_local[idx] = std::max(max_local[idx], max_private[idx]);
                    suma_cuadrados_local[idx] += suma2_private[idx];
                }
            }
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // FASE 2: REDUCCIÓN MPI
    // ═══════════════════════════════════════════════════════════════════════
    
    std::vector<float> min_global(total_dims);
    std::vector<float> max_global(total_dims);
    std::vector<double> suma_cuadrados_global(total_dims);
    std::vector<int> conteo_global(num_clusters);
    
    MPI_Reduce(min_local.data(), min_global.data(), total_dims, 
               MPI_FLOAT, MPI_MIN, 0, MPI_COMM_WORLD);
    
    MPI_Reduce(max_local.data(), max_global.data(), total_dims, 
               MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD);
    
    MPI_Reduce(suma_cuadrados_local.data(), suma_cuadrados_global.data(), total_dims, 
               MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    MPI_Reduce(conteo_local.data(), conteo_global.data(), num_clusters, 
               MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    // ═══════════════════════════════════════════════════════════════════════
    // FASE 3: CÁLCULO Y DISPLAY (solo id 0)
    // ═══════════════════════════════════════════════════════════════════════
    
    if (id == 0) {
        /*
        std::cout << "\n";
        std::cout << "═══════════════════════════════════════════════════════════════\n";
        std::cout << "                   ESTADÍSTICAS POR CLUSTER                    \n";
        std::cout << "═══════════════════════════════════════════════════════════════\n";
        std::cout << "\n";
        */
        for (int c = 0; c < num_clusters; ++c) {
            //std::cout << "Cluster " << c << " (" << conteo_global[c] << " puntos)\n";
            //std::cout << std::string(65, '-') << "\n";
            
            if (conteo_global[c] == 0) {
                //std::cout << "  [Cluster vacío]\n\n";
                continue;
            }
            /*
            std::cout << std::setw(6) << "Dim" 
                      << std::setw(15) << "Min"
                      << std::setw(15) << "Max"
                      << std::setw(15) << "Media"
                      << std::setw(15) << "Varianza"
                      << "\n";
            std::cout << std::string(65, '-') << "\n";
            */
            
            for (size_t d = 0; d < n_columnas; ++d) {
                size_t idx = c * n_columnas + d;
                
                float min_val = min_global[idx];
                float max_val = max_global[idx];
                float media = centroides[c * n_columnas + d];  // ¡Gratis!
                
                // Varianza = E[X²] - (E[X])²
                double varianza = (suma_cuadrados_global[idx] / conteo_global[c]) 
                                  - (static_cast<double>(media) * media);
                
                // Corregir error numérico
                if (varianza < 0.0 && varianza > -1e-10) {
                    varianza = 0.0;
                }
                /*
                std::cout << std::setw(6) << d << " "
                          << std::setw(15) << min_val << " "
                          << std::setw(15) << max_val << " "
                          << std::setw(15) << media << " "
                          << std::setw(15) << varianza
                          << "\n";
                */
            }
            
            //std::cout << "\n";
        }
        
        //std::cout << "═══════════════════════════════════════════════════════════════\n\n";
    }
}



/**
 * @brief Versión que devuelve el struct con las estadísticas.
 * @param datos_local           el vector con los datos
 * @param n_filas_local         el número de puntos
 * @param n_columnas            el número de coordenadas
 * @param centroides            el vector con los centroides
 * @param asignaciones_local    el vector con los centroides
 * @param num_cluesters         el número de clústeres
 */
Estadisticas calcularEstadisticasMPI(
    const std::vector<float>& datos_local,
    size_t n_filas_local,
    size_t n_columnas,
    const std::vector<float>& centroides,
    const std::vector<int>& asignaciones_local, 
    int num_clusters)
{
    int id, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    size_t total_dims = num_clusters * n_columnas;
    
    //puedo tener un struct para esto pero dudo que lo haga más rapido
    std::vector<float> min_local(total_dims, std::numeric_limits<float>::max());
    std::vector<float> max_local(total_dims, std::numeric_limits<float>::lowest());
    std::vector<double> suma_cuadrados_local(total_dims, 0.0);
    std::vector<int> conteo_local(num_clusters, 0);
    
    // Paralelización OpenMP con copias privadas
    #pragma omp parallel
    {
        //creo una copia local de cada una de las estructuras
        std::vector<float> min_private(total_dims, std::numeric_limits<float>::max());
        std::vector<float> max_private(total_dims, std::numeric_limits<float>::lowest());
        std::vector<double> suma2_private(total_dims, 0.0);
        std::vector<int> conteo_private(num_clusters, 0);
        
        #pragma omp for nowait schedule(static) //static es caché friendly
        for (size_t i = 0; i < n_filas_local; ++i) {
            int cluster = asignaciones_local[i];
            const float* punto = &datos_local[i * n_columnas];
            
            conteo_private[cluster]++;
            
            size_t base_idx = cluster * n_columnas;
            
            for (size_t d = 0; d < n_columnas; ++d) {
                float valor = punto[d];
                size_t idx = base_idx + d;
                
                min_private[idx] = std::min(min_private[idx], valor);
                max_private[idx] = std::max(max_private[idx], valor);
                suma2_private[idx] += static_cast<double>(valor) * valor;
            }
        }
        

        //aquí hay un cuello de botella que flipas loco
        //combinar los resultados, crítical no es óptimo tengo que buscar la manera de paralelizarlo
        #pragma omp critical
        {
            for (int c = 0; c < num_clusters; ++c) {
                conteo_local[c] += conteo_private[c];
                
                size_t base_idx = c * n_columnas;
                for (size_t d = 0; d < n_columnas; ++d) {
                    size_t idx = base_idx + d;
                    min_local[idx] = std::min(min_local[idx], min_private[idx]);
                    max_local[idx] = std::max(max_local[idx], max_private[idx]);
                    suma_cuadrados_local[idx] += suma2_private[idx];
                }
            }
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════
    // FASE 2: REDUCCIÓN MPI
    // ═══════════════════════════════════════════════════════════════════════
    
    std::vector<float> min_global(total_dims);
    std::vector<float> max_global(total_dims);
    std::vector<double> suma_cuadrados_global(total_dims);
    std::vector<int> conteo_global(num_clusters);
    
    MPI_Reduce(min_local.data(), min_global.data(), total_dims, 
               MPI_FLOAT, MPI_MIN, 0, MPI_COMM_WORLD);
    
    MPI_Reduce(max_local.data(), max_global.data(), total_dims, 
               MPI_FLOAT, MPI_MAX, 0, MPI_COMM_WORLD);
    
    MPI_Reduce(suma_cuadrados_local.data(), suma_cuadrados_global.data(), total_dims, 
               MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    MPI_Reduce(conteo_local.data(), conteo_global.data(), num_clusters, 
               MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    //soy el nodo cero entonces creo el dataset y lo devuelvo
    if (id == 0) {
        Estadisticas st = 
        for (int c = 0; c < num_clusters; ++c) {
            //std::cout << "Cluster " << c << " (" << conteo_global[c] << " puntos)\n";
            //std::cout << std::string(65, '-') << "\n";
            
            if (conteo_global[c] == 0) {
                //std::cout << "  [Cluster vacío]\n\n";
                continue;
            }
            
            for (size_t d = 0; d < n_columnas; ++d) {
                size_t idx = c * n_columnas + d;
                
                float min_val = min_global[idx];
                float max_val = max_global[idx];
                float media = centroides[c * n_columnas + d];  // ¡Gratis!
                
                // Varianza = E[X²] - (E[X])²
                double varianza = (suma_cuadrados_global[idx] / conteo_global[c]) 
                                  - (static_cast<double>(media) * media);
                
                // Corregir error numérico
                if (varianza < 0.0 && varianza > -1e-10) {
                    varianza = 0.0;
                }
            }
        }
    }
}