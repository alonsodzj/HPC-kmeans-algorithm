//dependencias locales
#include "dependencies/lectura.h"  
#include "dependencies/kmeans.h"
#include "dependencies/estadisticas_parallel.h"

//dependencias paralelas
#include "mpi.h"
#include "omp.h"



int main(int argc, char** argv){
    
    MPI_Init (&argc,&argv); //Inicio MPI

    size_t id, size;    //identificador del nodo y número total de nodos

    MPI_Comm_rank(MPI_COMM_WORLD, &id);     //obtengo el id del cluster
    MPI_Comm_size(MPI_COMM_WORLD, &size);   //obtengo el tamaño del comunicador (número de nodos)

    //compruebo que es el maestro para poder leer mi dataset.
    
    std::vector<float> datos;   //puntero al vector donde voy a almacenar los datos, cada nodo crea 1

    size_t n_filas = 0, n_columnas = 0;
    
    if(id==0)   //soy el maestro, entonces leo del dataset para luego repartir
    {
        const char* nombre_archivo = "data/salida.bin";
        std::cout<<"nodo " << id << " leyendo archivos...\n";
        
        leerDatos(nombre_archivo, datos, n_filas, n_columnas);
        if (datos.empty()) {
            std::cerr << "No se pudieron leer los datos del archivo.\n";
            return 1;
        }
        
        std::cout << "Número de puntos: " << n_filas << "\n";
        std::cout << "Número de coordenadas: " << n_columnas<< "\n";
    
    }
    std::cout << std::fixed << std::setprecision(9); //esto es para la precisión de impresión

    //reparto las filas y las columnas desde el nodo 0, 
    //para que el resto lea deben ejecutar el código, por eso lo pongo fuera.
    MPI_Bcast(&n_filas, 1, MPI_AINT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n_columnas, 1, MPI_AINT, 0, MPI_COMM_WORLD);
    
    //reservo un vector local de datos con el número de puntos correspondientes.
    size_t n_filas_local = n_filas / size;     
    std::vector<float> datos_local(n_filas_local*n_columnas);   //creo el dataset con el que voy a llamar a mi función kmedias.


    Dataset dataset(std::move(datos), numPuntos, numCoords);
    std::vector<float> centroides(NUM_CENTROIDES*numCoords);  //como los centroides son puntos entonces tienen las mismas dimensiones que estos
    std::vector<int> asignaciones(dataset.numPuntos);




    std::cout << "ejecutando algoritmo... \n";
    double kmeans_0 = omp_get_wtime();   //para calcular el tiempo
    
    //tengo que hacer una llamada por nodo con los datos asignados a este.
    kmeans(dataset, centroides, asignaciones);

    //para calcular el tiempo
    double kmeans_1 = omp_get_wtime(); 
    double tiempo = kmeans_1 - kmeans_0;
    std::cout << "kmeans calculado en: " << tiempo << " segundos\n";

    double est_0 = omp_get_wtime(); //para calcular el tiempo

    calcularEstadisticas(dataset,asignaciones); //llamo a la función que me calcula las estadísticas de mis datos con la menor cantidad de pasadas posibles.
    
    //para calcular el tiempo
    double est_1 = omp_get_wtime();
    tiempo = est_1 - est_0;
    std::cout << "Estadísticas calculadas en: " << tiempo << " segundos\n";
    

    //finalizo openMPI
    MPI_Finalize()
}