#pragma once

#include "dependencies/kmeans.h"
#include "dependencies/binreader.h"

#define NUM_CENTROIDES 5;

int main(){
    //primero leo los datos del archivo binario que me devuelve un vector de vectores con los puntos
    reader rd;
    std::vector<float> datos = rd.leerDatos("data/salida.bin"); //vector de datos
    int numCoords = rd.getNumCoords();                              //numero de coordenadas
    int numPuntos = rd.getNumPuntos();                              //numero de puntos
    //--CREAR DATASET--
    Dataset dataset(std::move(datos), numPuntos, numCoords);
    //--AHORA LLAMO AL ALGORITMO--
    std::vector<float> centroides(NUM_CENTROIDES * numCoords);  //como los centroides son puntos entonces tienen las mismas dimensiones que estos
    std::vector<int> asignaciones(dataset.numPuntos);
    kmeans(dataset, centroides, asignaciones);                  //llamo a mi algoritmo y luego realizo las operaciones para lso diferentes datos.
    //AQUÍ ESTÁN LOS CENTROIDES FINALES
}