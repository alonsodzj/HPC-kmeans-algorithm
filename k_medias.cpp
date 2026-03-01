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

int main()
{
    //aquí se implementa mi algoritmo k medias.
}


Algoritmo KMeans(datos, K, max_iteraciones):

    // datos: lista de puntos
    // K: número de clusters

    Inicializar K centroides aleatoriamente

    Para iteracion desde 1 hasta max_iteraciones:

        // Paso 1: Asignación
        Para cada punto en datos:
            calcular distancia a cada centroide
            asignar punto al centroide más cercano

        // Paso 2: Actualización
        Para cada cluster:
            recalcular centroide como promedio de sus puntos

        Si los centroides no cambian:
            romper el ciclo

    retornar centroides y asignaciones

struct Punto {
    double x, y;
    int cluster;
};

double distancia(Punto a, Punto b) {
    return sqrt((a.x - b.x)*(a.x - b.x) +
                (a.y - b.y)*(a.y - b.y));
}

KMeans(vector<Punto>& datos, int K) {

    Inicializar centroides aleatoriamente

    bool cambio = true;

    while (cambio) {

        cambio = false;

        // Asignación
        for (cada punto p en datos) {
            encontrar centroide más cercano
            si cluster cambia:
                cambio = true
        }

        // Actualización
        para cada cluster:
            calcular nuevo centroide (promedio)
    }
}