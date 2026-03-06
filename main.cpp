#pragma once

#include "dependencies/kmeans.h"
#include "dependencies/binreader.h"

#define NUM_CENTROIDES 5


float minima(Dataset data, std::vector<int> asignaciones, int groupID)
{

}
float maxima(Dataset data,std::vector<int> asignaciones, int groupID)
{

}
float media(Dataset data,std::vector<int> asignaciones, int groupID)
{

}
float varianza(Dataset data,std::vector<int> asignaciones, int groupID)
{

}

int main(){
    //primero leo los datos del archivo binario que me devuelve un vector de vectores con los puntos
    const char* archivo = "salida.bin";
    reader rd;
    std::vector<float> datos = rd.leerDatos(archivo); //vector de datos
    if (datos.empty()) {
        std::cerr << "No se pudieron leer los datos del archivo.\n";
        return 1;
    }
    int numCoords = rd.getNumCoords();              //numero de coordenadas
    int numPuntos = rd.getNumPuntos();              //numero de puntos
    std::cout << "Número de filas: " << rd.getNumPuntos() << "\n";
    std::cout << "Número de columnas: " << rd.getNumCoords() << "\n";
    //--CREAR DATASET--
    Dataset dataset(std::move(datos), numPuntos, numCoords);
    //--AHORA LLAMO AL ALGORITMO--
    std::vector<float> centroides(NUM_CENTROIDES*numCoords);  //como los centroides son puntos entonces tienen las mismas dimensiones que estos
    std::vector<int> asignaciones(dataset.numPuntos);
    kmeans(dataset, centroides, asignaciones);                  //llamo a mi algoritmo y luego realizo las operaciones para lso diferentes datos.
    //AQUÍ ESTÁN LOS CENTROIDES FINALES
}