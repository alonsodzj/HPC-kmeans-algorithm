#pragma once

#include "mpi.h"
#include "omp.h"
#include <vector>
#include <iostream>
#include <iomanip>
#include <limits>
#include <cmath>
#include <algorithm>


//este archivo es igual que el de estadisticas_mpi.h pero lo que hace es quitarse la parte critical al hacer la reducción
//hace todo con un reduction pero para usarlo necesito un puntero a los datos de las diferentes estructuras

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
    
    //creo las diferentes estructuras
    std::vector<float>  min_local(total_dims, std::numeric_limits<float>::max());
    std::vector<float>  max_local(total_dims, std::numeric_limits<float>::lowest());
    std::vector<double> suma_cuadrados_local(total_dims, 0.0);
    std::vector<int>    conteo_local(num_clusters, 0);

    //creo punteros a los datos de las diferentes estructuras
    float*  min_ptr    = min_local.data();
    float*  max_ptr    = max_local.data();
    double* suma_ptr   = suma_cuadrados_local.data();
    int*    conteo_ptr = conteo_local.data();

    #pragma omp parallel for \
        reduction(+:conteo_ptr[:num_clusters], suma_ptr[:total_dims]) \
        reduction(min:min_ptr[:total_dims]) \
        reduction(max:max_ptr[:total_dims]) \
        schedule(static)
    for (size_t i = 0; i < n_filas_local; ++i) {

        int cluster = asignaciones_local[i];
        const float* punto = &datos_local[i * n_columnas];
        
        conteo_ptr[cluster]++;
        
        size_t base_idx = cluster * n_columnas;
        
        #pragma omp simd
        for (size_t d = 0; d < n_columnas; ++d) {
            float valor = punto[d];
            size_t idx = base_idx + d;
            
            min_ptr[idx] = std::min(min_ptr[idx], valor);
            max_ptr[idx] = std::max(max_ptr[idx], valor);
            suma_ptr[idx] += static_cast<double>(valor) * valor;
        }
    }
    
    std::vector<float>  min_global(total_dims);
    std::vector<float>  max_global(total_dims);
    std::vector<double> suma_cuadrados_global(total_dims);
    std::vector<int>    conteo_global(num_clusters);
    
    MPI_Reduce(min_ptr,  min_global.data(),  total_dims,   MPI_FLOAT,  MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(max_ptr,  max_global.data(),  total_dims,   MPI_FLOAT,  MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(suma_ptr, suma_cuadrados_global.data(), total_dims, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(conteo_ptr, conteo_global.data(), num_clusters, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (id == 0) {
        for (int c = 0; c < num_clusters; ++c) {
            if (conteo_global[c] == 0) continue;
            
            for (size_t d = 0; d < n_columnas; ++d) {
                size_t idx = c * n_columnas + d;
                
                float  media   = centroides[c * n_columnas + d];
                double varianza = (suma_cuadrados_global[idx] / conteo_global[c])
                                - (static_cast<double>(media) * media);
                
                if (varianza < 0.0 && varianza > -1e-10) varianza = 0.0;
            }
        }
    }
}