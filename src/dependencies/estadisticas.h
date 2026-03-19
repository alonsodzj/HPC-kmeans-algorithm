#pragma once

#include "mpi.h"
#include "omp.h"
#include <vector>
#include <iostream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <algorithm>

void calcularEstadisticasMPI(
    const std::vector<float>& datos_local,  //el vector con los datos
    size_t n_filas_local,                   //el número de puntos
    size_t n_columnas,                      //el número de columnas
    const std::vector<float>& centroides,   //el vector de centroides
    const std::vector<int>& asignaciones_local, //el vector de asignaciones
    int num_clusters)                       //el número de clústeres
{
    int id, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //el tamaño total de los vectores que tengo que crear
    size_t total_dims = num_clusters * n_columnas;
    
    std::vector<float> min_local(total_dims, std::numeric_limits<float>::max());
    std::vector<float> max_local(total_dims, std::numeric_limits<float>::lowest());
    std::vector<double> suma_cuadrados_local(total_dims, 0.0);
    std::vector<int> conteo_local(num_clusters, 0);
    
    // ═══════════════════════════════════════════════════════════════════
    // CÁLCULO LOCAL PARALELO (SIN CRITICAL NI ATOMICS - USANDO REDUCTION)
    // ═══════════════════════════════════════════════════════════════════
    
    //uso la concatenación de reduciones para poder ejecutar mi algoritmo de una forma muy eficiente.
    #pragma omp parallel for \
        reduction(+:conteo_local[:num_clusters], suma_cuadrados_local[:total_dims]) \
        reduction(min:min_local[:total_dims]) \
        reduction(max:max_local[:total_dims]) \
        schedule(static)    //esto mejora la localidad de la caché
    for (size_t i = 0; i < n_filas_local; ++i) {

        int cluster = asignaciones_local[i];    //el clúster es el grupo al que pertenece el punto.
        const float* punto = &datos_local[i * n_columnas];  //creo una referencia al punto para mejorar la localidad de los datos
        
        conteo_local[cluster]++;
        
        size_t base_idx = cluster * n_columnas;
        
        #pragma omp simd
        for (size_t d = 0; d < n_columnas; ++d) {
            float valor = punto[d];
            size_t idx = base_idx + d;
            
            min_local[idx] = std::min(min_local[idx], valor);
            max_local[idx] = std::max(max_local[idx], valor);
            suma_cuadrados_local[idx] += static_cast<double>(valor) * valor;
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════
    // REDUCCIÓN MPI
    // ═══════════════════════════════════════════════════════════════════
    
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
    
    // ═══════════════════════════════════════════════════════════════════
    // CÁLCULO FINAL (solo rank 0)
    // ═══════════════════════════════════════════════════════════════════
    
    if (id == 0) {
        for (int c = 0; c < num_clusters; ++c) {
            if (conteo_global[c] == 0) continue;
            
            for (size_t d = 0; d < n_columnas; ++d) {
                size_t idx = c * n_columnas + d;
                
                float min_val = min_global[idx];
                float max_val = max_global[idx];
                float media = centroides[c * n_columnas + d];
                
                double varianza = (suma_cuadrados_global[idx] / conteo_global[c]) 
                                  - (static_cast<double>(media) * media);
                
                if (varianza < 0.0 && varianza > -1e-10) {
                    varianza = 0.0;
                }
            }
        }
    }
}