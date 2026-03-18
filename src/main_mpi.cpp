//dependencias locales
#include "dependencies/constantes.h"
#include "dependencies/lectura.h"  
#include "dependencies/kmeans_mpi.h"
#include "dependencies/estadisticas_mpi.h"


//dependencias paralelas
#include "mpi.h"
#include "omp.h"

#include <iostream>
#include <iomanip>
#include <vector>

int main(int argc, char** argv) {
    //inicio MPI
    MPI_Init(&argc, &argv);

    int id, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::cout << std::fixed << std::setprecision(9);    //Esto es para la precisión

    std::vector<float> datos;
    size_t n_filas = 0, n_columnas = 0;
    
    if (id == 0) {
        const char* nombre_archivo = "../data/salida.bin";
        std::cout << "Nodo " << id << " leyendo archivos...\n";
        
        leerDatos(nombre_archivo, datos, n_filas, n_columnas);
        
        if (datos.empty()) {
            std::cerr << "No se pudieron leer los datos del archivo.\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
        
        std::cout << "Número de puntos: " << n_filas << "\n";
        std::cout << "Número de coordenadas: " << n_columnas << "\n";
        std::cout << "Número de procesos MPI: " << size << "\n";
    }
    
    //una vez he leído los datos mando el número de filas y el número de columnas a cada nodo
    MPI_Bcast(&n_filas, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n_columnas, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
    
    // ═══════════════════════════════════════════════════════════════════════
    // DISTRIBUCIÓN DE DATOS CON SCATTERV (maneja tamaños desiguales)
    // ═══════════════════════════════════════════════════════════════════════
    
    // Calcular cuántos puntos le corresponden a cada proceso
    size_t puntos_por_proceso = n_filas / size;
    size_t puntos_restantes = n_filas % size;
    
    // Este proceso recibirá:
    size_t n_filas_local = puntos_por_proceso;
    if (id < static_cast<int>(puntos_restantes)) {
        n_filas_local++;  // Los primeros 'puntos_restantes' procesos reciben 1 extra
    }
    
    // Preparar buffers para Scatterv
    std::vector<int> sendcounts(size);
    std::vector<int> displs(size);
    
    if (id == 0) {
        size_t offset = 0;
        for (int r = 0; r < size; ++r) {
            size_t n_local = puntos_por_proceso;
            if (r < static_cast<int>(puntos_restantes)) {
                n_local++;
            }
            sendcounts[r] = n_local * n_columnas;
            displs[r] = offset * n_columnas;
            offset += n_local;
        }
    }
    
    // Reservar espacio local
    std::vector<float> datos_local(n_filas_local * n_columnas);
    
    // Distribuir datos desde rank 0 a todos
    MPI_Scatterv(
        datos.data(),           // Buffer de envío (solo el nodo 0 lo usa)
        sendcounts.data(),      // Cuántos elementos enviar a cada proceso
        displs.data(),          // Desplazamiento en el buffer de envío
        MPI_FLOAT,
        datos_local.data(),     // Buffer de recepción (todos)
        n_filas_local * n_columnas,  // Cuántos elementos recibir
        MPI_FLOAT,
        0,                      // Root (quien envía)
        MPI_COMM_WORLD
    );
    
    // Liberar memoria en el nodo 0 (ya no necesita el dataset completo)
    if (id == 0) {
        datos.clear();
        datos.shrink_to_fit();
    }
    
    //inicializo los centroides
    std::vector<float> centroides(NUM_CENTROIDES * n_columnas);
    if (id == 0) {
        centroides.resize(NUM_CENTROIDES * n_columnas);
        
        // Inicialización: seleccionar primeros NUM_CENTROIDES puntos
        // (Alternativamente: k-means++, random, etc.)
        for (int i = 0; i < NUM_CENTROIDES && i < static_cast<int>(n_filas_local); ++i) {
            for (size_t j = 0; j < n_columnas; ++j) {
                centroides[i * n_columnas + j] = datos_local[i * n_columnas + j];
            }
        }
    }
    
    // Broadcast de centroides iniciales
    MPI_Bcast(centroides.data(), NUM_CENTROIDES * n_columnas, MPI_FLOAT, 0, MPI_COMM_WORLD);
    
    // Vector de asignaciones local
    std::vector<int> asignaciones_local;
    
    //ejecución del algoritmo kmeans

    //espero a que lleguen todos los nodos aquí para medir cuanto tarda en ejecutarse el algoritmo.
    MPI_Barrier(MPI_COMM_WORLD);

    if (id == 0) {
        std::cout << "\nEjecutando algoritmo K-Means distribuido...\n";
    }
    double kmeans_start = MPI_Wtime();

    if (id == 0) {
        #pragma omp parallel
        {
            #pragma omp master
            {
                std::cout << "Threads OpenMP disponibles: " << omp_get_num_threads() << "\n";
            }
        }
    }

    //hago la llamada a mi algoritmo paralelo con el dataset local, las asignaciones locales y el resto de argumentos
    kmeans_mpi(
        datos_local, 
        n_filas_local, 
        n_columnas, 
        centroides,           // Se actualiza in-place
        asignaciones_local    // Se llena con las asignaciones
    );
    
    //espero a que todos los nodos lleguen de nuevo
    MPI_Barrier(MPI_COMM_WORLD);

    double kmeans_end = MPI_Wtime();
    if (id == 0) {
        double tiempo_kmeans = kmeans_end - kmeans_start;
        std::cout << "K-Means calculado en: " << tiempo_kmeans << " segundos\n";
    }
    
    //calcular las estadísticas

    //espero a que lleguen de nuevo para realizar la medición correctamente.
    MPI_Barrier(MPI_COMM_WORLD);

    if (id == 0) {
        std::cout << "\nCalculando estadísticas distribuidas...\n";
    }
    double stats_start = MPI_Wtime();
    
    //llamo a la función que me calcula las estadísticas de una sola pasada
    calcularEstadisticasMPI(
        datos_local,         // Datos locales de este proceso
        n_filas_local,       // Número de puntos locales
        n_columnas,          // número de coordenadas
        centroides,          // Centroides finales (contienen las medias)
        asignaciones_local,  // Asignaciones locales
        NUM_CENTROIDES       // Número de clusters
    );
    
    //espero a que todos lleguen 
    MPI_Barrier(MPI_COMM_WORLD);

    double stats_end = MPI_Wtime();
    if (id == 0) {
        double tiempo_stats = stats_end - stats_start;
        std::cout << "Estadísticas calculadas en: " << tiempo_stats << " segundos\n";
    }
    
    //termino MPI
    MPI_Finalize();
    return 0;
}