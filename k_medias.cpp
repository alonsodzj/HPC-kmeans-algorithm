//file para implementar el algoritmo k medias para mi programa.
//luego lo implemento de manera paralela con openmp y mpi
#include <vector>
#include <iostream>
#include <cmath>
#include "binreader.h"
#include <omp.h>

//algoritmo termina cuando los centroides no cambian o cuando se alcanza un número máximo de iteraciones.
//también termina si conseguimos la calidad deseada, es decir, si la suma de las distancias de los puntos a sus centroides es menor que un umbral dado.

//debido a la inicialización aleatoria de los centroides, el algoritmo puede converger a un mínimo local, por lo que es común ejecutar el algoritmo varias veces con diferentes inicializaciones y elegir la mejor solución.

//debido a que se usa la distancia euclidia, no es adecuado para formas no esféricas


double distancia(std::vector<float>& punto1, std::vector<float>& punto2)
/*
función para calcular la distancia entre dos puntos, esto es necesario para
asignar los puntos a los centroides y para calcular la calidad del resultado.
*/
{
    //esto normalmente se calcula con la raíz cuadrada de la suma de las diferencias al cuadrado, pero para evitar cálculos innecesarios, podemos comparar las distancias al cuadrado directamente.
    double distTotal = 0.0;
    for (int i = 0; i < punto1.size(); i++)
    {
        //por cada coordenada, calculo la diferencia al cuadrado y la sumo a la distancia total.
        distTotal += std::pow(punto1[i] - punto2[i], 2);
    }
    return distTotal;   //retorno la distancia total al cuadrado. //esto me sirve para comparar las distancias sin necesidad de calcular la raíz cuadrada, lo que mejora el rendimiento.
}

double distanciaEnUnaDimension(std::vector<float>& punto1, std::vector<float>& punto2, int dimension)
{
    //calculo la distancia en una dimensión específica, esto es útil para el cálculo de los centroides.
    return std::pow(punto1[dimension] - punto2[dimension], 2);   //retorno la distancia al cuadrado en la dimensión especificada.
}

int main()
{
    //primero leo los datos del archivo binario que me devuelve un vector de vectores con los puntos
     
    auto datos = leerDatosBinarios("salida.bin");
    int numCoords = getNumCoords(datos);    //leo el número de coordenadas
    int numPuntos = getNumPuntos(datos);    //leo el número de puntos (filas)

    //aquí más tarde podré realizar la distribución de carga.


}
