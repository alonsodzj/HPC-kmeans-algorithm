#include "dependencies/binreader.h"
#include <vector>
#include <iostream>
#include <cmath>
#include <omp.h>

//algoritmo termina cuando los centroides no cambian o cuando se alcanza un número máximo de iteraciones.
//también termina si conseguimos la calidad deseada, es decir, si la suma de las distancias de los puntos a sus centroides es menor que un umbral dado.

//debido a la inicialización aleatoria de los centroides, el algoritmo puede converger a un mínimo local, por lo que es común ejecutar el algoritmo varias veces con diferentes inicializaciones y elegir la mejor solución.

//debido a que se usa la distancia euclidia, no es adecuado para formas no esféricas
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
    return distTotal;   //retorno la distancia total al cuadrado. //esto me sirve para comparar las distancias sin necesidad de calcular la raíz cuadrada, lo que mejora el rendimiento.
}

float distanciaEnUnaDimension(std::vector<float>&datos, int index1, int index2, int dimension){
    return std::pow(datos[index1*numCoords+dimension] - datos[index2*numCoords+dimension], 2);
}

int main()
{
    //primero leo los datos del archivo binario que me devuelve un vector de vectores con los puntos
    reader rd;
    std::vector<float> datos = rd.leerDatos("salida.bin");  //vector de datos
    numCoords = rd.getNumCoords();                          //numero de coordenadas
    numPuntos = rd.getNumPuntos();                          //numero de puntos
    //a partir de aquí ya tengo la información necesaria para realizar el algoritmo.

    //aquí más tarde podré realizar la distribución de carga.


}
