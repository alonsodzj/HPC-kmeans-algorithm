#include "dependencies/binreader.h"
#include "dependencies/kmeans.h"
#include "dependencies/binreader.h"
//#include "dependencies/estadisticas.h"
#include "dependencies/estadisticas_parallel.h"
#include "dependencies/guardar.h"
#include "omp.h"

int main(){
    std::cout << std::fixed << std::setprecision(6); // 6 decimales fijos
    std::cout<<"leyendo archivos...\n";
    //primero leo los datos del archivo binario que me devuelve un vector de vectores con los puntos
    const char* archivo = "data/salida.bin";
    reader rd;
    std::vector<float> datos = rd.leerDatos(archivo); //vector de datos
    if (datos.empty()) {
        std::cerr << "No se pudieron leer los datos del archivo.\n";
        return 1;
    }
    int numCoords = rd.getNumCoords();              //numero de coordenadas
    int numPuntos = rd.getNumPuntos();              //numero de puntos
    std::cout << "Número de puntos: " << rd.getNumPuntos() << "\n";
    std::cout << "Número de coordenadas: " << rd.getNumCoords() << "\n";
    //--CREAR DATASET--

    Dataset dataset(std::move(datos), numPuntos, numCoords);
    std::vector<float> centroides(NUM_CENTROIDES*numCoords);  //como los centroides son puntos entonces tienen las mismas dimensiones que estos
    std::vector<int> asignaciones(dataset.numPuntos);

    std::cout << "ejecutando algoritmo... \n";
    //--AHORA LLAMO AL ALGORITMO--
    double kmeans0 = omp_get_wtime();
    kmeans(dataset, centroides, asignaciones);      //llamo a mi algoritmo y luego realizo las operaciones para lso diferentes datos.
    double kmeans1 = omp_get_wtime();
    double tiempo = kmeans1 - kmeans0;
    std::cout << "kmeans calculado en: " << tiempo << " segundos\n";

    double estadisticas0 = omp_get_wtime();

    calcularEstadisticas(dataset,asignaciones);     //llamo a la función que me calcula las estadísticas de mis datos con la menor cantidad de pasadas posibles.
    
    double estadisticas1 = omp_get_wtime();
    tiempo = estadisticas1 - estadisticas0;
    std::cout << "Estadísticas calculadas en: " << tiempo << " segundos\n";
    
    guardarCentroides(centroides,numCoords);
}