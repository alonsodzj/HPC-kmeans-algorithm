#include "dependencies/binreader.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <omp.h>

#define NUM_GRUPOS      20; //cada grupo teiene unos centroides asignados.
#define NUM_CLUSTERS    5;  //cada clúster tiene unos grupos asignados
#define GROUPS_BY_CLSUTER = NUM_GRUPOS / NUM_CLUSTERS;



//algoritmo termina cuando los centroides no cambian o cuando se alcanza un número máximo de iteraciones.
//también termina si conseguimos la calidad deseada, es decir, si la suma de las distancias de los puntos a sus centroides es menor que un umbral dado.
int numPuntos = 0;
int numCoords = 0;

double distancia(std::vector<float>&datos, int index1, int index2)
{
    double distTotal = 0.0;
    for (int i = 0; i < numCoords; i++)
    {
        //por cada coordenada, calculo la diferencia al cuadrado y la sumo a la distancia total.
        distTotal += std::pow(datos[index1*numPuntos+i] - datos[index2*numPuntos+i], 2);
    }
    return distTotal;
}

float distanciaEnUnaDimension(std::vector<float>&datos, int index1, int index2, int dimension)
{
    return std::pow(datos[index1*numCoords+dimension] - datos[index2*numCoords+dimension], 2);
}

int main()
{
    //primero leo los datos del archivo binario que me devuelve un vector de vectores con los puntos
    reader rd;
    std::vector<float> datos = rd.leerDatos("data/salida.bin"); //vector de datos
    numCoords = rd.getNumCoords();                              //numero de coordenadas
    numPuntos = rd.getNumPuntos();                              //numero de puntos
    //a partir de aquí ya tengo la información necesaria para realizar el algoritmo.

    //aquí más tarde podré realizar la distribución de carga.


}

void kmeans(const std::vector<float>&data, std::vector<float>& centroides, std::vector<int>&asignaciones)
{

}