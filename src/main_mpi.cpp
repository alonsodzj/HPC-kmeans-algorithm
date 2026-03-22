//dependencias locales
#include "dependencies/constantes.h"
#include "dependencies/lectura.h"  
#include "dependencies/kmeans_packed.h"
//#include "dependencies/kmeans_mpi.h"
//#include "dependencies/estadisticas_mpi"
#include "dependencies/estadisticas.h"


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

    //creo el vector de datos para la lectura y las variables para las filas y las columnas.
    std::vector<float> datos;
    size_t n_filas = 0;
    size_t n_columnas = 0;
    
    //si soy el nodo cero entonces leo
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
        std::cout << "Número de procesos MPI: " << size << "\n";    //imprimo el tamaño del comunicador
    }
    
    //una vez he leído los datos mando el número de filas y el número de columnas a cada nodo mediante un broadcast
    MPI_Bcast(&n_filas, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n_columnas, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
    
    // DISTRIBUCIÓN DE DATOS CON SCATTERV (maneja tamaños desiguales)

    // Calcular cuántos puntos le corresponden a cada proceso
    size_t puntos_por_proceso = n_filas / size;
    size_t puntos_restantes = n_filas % size;
    
    // Este proceso recibirá:
    size_t n_filas_local = puntos_por_proceso;
    if (id < static_cast<int>(puntos_restantes)) {
        n_filas_local++;  // Los primeros 'puntos_restantes' procesos reciben 1 extra
    }
    
    // Preparar buffers para Scatterv (hago siempre los buffers con std::vector)
    std::vector<int> sendcounts(size);  //cuantos elementos enviar a cada proceso 
    std::vector<int> displs(size);      //dónde empieza cada porción en el buffer original
    
    //solo lo calcula el nodo cero, los demás procesos solo tienen que recibir
    if (id == 0) {
        size_t offset = 0;  //índice de inicio del dataset para cada proceso
        for (int r = 0; r < size; ++r) 
        //para cada proceso
        {
            size_t n_local = puntos_por_proceso;
            if (r < static_cast<int>(puntos_restantes)) 
            //si el índice de mi nodo es menor a puntos restantes se le asigna uno extra
            {
                n_local++;
            }
            sendcounts[r] = n_local * n_columnas;   //se asigna para cada índice de nodo el tamaño de puntos a /enviar/recibir
            displs[r] = offset * n_columnas;    //índice de donde empieza cada proceso en el vector plano
            offset += n_local;  //se actualiza el offset para la siguiente iteración.
        }
    }
    
    //creo el vector de centroides que va a ser el mismo para todos los nodos
    std::vector<float> centroides(NUM_CENTROIDES * n_columnas);
    //reserva del buffer de recepcion para el scatter
    std::vector<float> datos_local(n_filas_local * n_columnas);
    
    if(id==0)
    {
        //selecciono de centroides puntso estratégicas de mi vector de datos
        size_t paso = n_filas / NUM_CENTROIDES;

        for (int i = 0; i < NUM_CENTROIDES; ++i) {
            size_t fila = i * paso;

            for (size_t j = 0; j < n_columnas; ++j) {
                centroides[i * n_columnas + j] = datos[fila * n_columnas + j];
            }
        }
    }
    
    // Distribuir datos desde el nodo 0 a todos
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
    // Broadcast de centroides iniciales
    MPI_Bcast(centroides.data(), NUM_CENTROIDES * n_columnas, MPI_FLOAT, 0, MPI_COMM_WORLD);
   

    // Liberar memoria en el nodo 0 (ya no necesita el dataset completo)
    //me cargo la información del buffer original porque ya tengo el dataset con la cantidad de datos deseada
    if (id == 0) 
    {
        datos.clear();
        datos.shrink_to_fit();
    }
     
    //espero a que lleguen todos los nodos aquí para medir cuanto tarda en ejecutarse el algoritmo.
    MPI_Barrier(MPI_COMM_WORLD);

    if (id == 0) {
        std::cout << "\nEjecutando algoritmo K-Means distribuido...\n";
    }
    double kmeans_start = MPI_Wtime();

    //esto es para cuestiones de depuración.
    if (id == 0) {
        #pragma omp parallel
        {
            #pragma omp master
            {
                std::cout << "Threads OpenMP disponibles: " << omp_get_num_threads() << "\n";
            }
        }
    }

    //creo el vector de asignaciones local
    std::vector<int> asignaciones_local;
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