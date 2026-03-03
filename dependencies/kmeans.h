#pragma once

#include <vector>
#include <iostream>
#include <cmath>
#include <omp.h>


//--STRUCT PARA ENCAPSULAR--
struct Dataset {
    std::vector<float> data;
    int numPuntos;
    int numCoords;
    // Constructor normal que copia
    Dataset(const std::vector<float>& datos, int puntos, int coords) : data(datos), numPuntos(puntos), numCoords(coords) {}
    // Constructor por defecto
    Dataset() : numPuntos(0), numCoords(0) {}
    // Constructor que mueve el vector (para eficiencia)
    Dataset(std::vector<float>&& datos, int puntos, int coords) : data(std::move(datos)), numPuntos(puntos), numCoords(coords) {}
};

//--FUNCIONES DE ACTUALIZACIÓN--
//aquí solo recalculo los centroides en base a los puntos de cada grupo
void actualizarCentroides(const Dataset& dataset, std::vector<float>& centroides, std::vector<int>& asignaciones)
{
    int numCentroides = centroides.size()/dataset.numCoords;

    std::vector<float> suma(centroides.size(), 0.0f); //vector de tamaño centroides para almacenar las sumas
    std::vector<int> conteo(numCentroides,0);           //vector de conteo inicializado a 0

    for (int i = 0; i < asignaciones.size(); i++)
    {
        //llevamos la cuenta de cuantos puntos pertenecen a cuantos centroides
        int centroide = asignaciones[i];
        conteo[centroide]++;
        for (int j = 0; j < dataset.numCoords; j++)
        {
            //por cada coordenada sumo el valor en cada centroide
            suma[centroide*dataset.numCoords+j] += dataset.data[i*dataset.numCoords+j];
        }
    }
    //en este punto ya tengo todos los puntos sumados, ahora tengo que actualizar suma dividiendo entre conteo para hacer la media
    //después tan solo actualizo centroides con el valor de suma.
    for (int i = 0; i < numCentroides; i++)
    {
        if(conteo[i]!=0)    //evitamos la división entre 0
        {
            for (int j = 0; j < dataset.numCoords; j++)
            {
                suma[i*dataset.numCoords+j] /= conteo[i];
                centroides[i*dataset.numCoords+j] = suma[i*dataset.numCoords+j];
            }
        }   
        //en caso de querer reubicar mi centroide perdido porque no tiene puntos asignados pudeo asignarle una posición random
    }
    
}
//aquí solo recalculo los grupos en base a los puntos de cada grupo
float actualizarGrupos(const Dataset& dataset, std::vector<float>& centroides, std::vector<int>& asignaciones)
{
    int numCentroides = centroides.size()/dataset.numCoords;
    float porcentajeDesplazados = 0.0f;
    //recorremos cada punto del dataset
    for (int i = 0; i < dataset.numPuntos; i++)
    {
        //recorro el dataset.
        int mejorCentroide = asignaciones[i];   
        //usamos esta variable para mantenerla en los registros y operar más rápido que asignarlo al vector (llamada a RAM o caché en el mejor caso)
        float menorDistancia = -1.0f;

        //por cada centroide calculo la distancia **futura paralelización**
        for (int j = 0; j < numCentroides; j++)
        {
            double distanciaAcumulada = 0.0f;
            for (int d = 0; d < dataset.numCoords; d++)
            {
                float diferencia = dataset.data[i * dataset.numCoords + d] - centroides[j * dataset.numCoords + d];
                distanciaAcumulada += diferencia * diferencia;
            }
            if (j==0 || distanciaAcumulada<menorDistancia)
            {
                //actualizamos
                menorDistancia=distanciaAcumulada;
                mejorCentroide = j; //d
            }
        }
        asignaciones[i]=mejorCentroide; //actualizo asignaciones.
    }
    return porcentajeDesplazados;
}

//--FUNCIÓN KMEANS SECUENCIAL--
void kmeans(const Dataset& dataset, std::vector<float>& centroides, std::vector<int>&asignaciones)
{
    //asigno los puntos a un determinado grupo antes de calcular los cenotroides
    for(int i = 0; i<asignaciones.size(); i++){
        asignaciones[i] = i % (centroides.size()/dataset.numCoords);
    }
    bool calidad = false;   //si mis puntos se desplazan menos del 5% entonces calidad ==true y mi algoritmo termina.
    int iteraciones = 0;    //si supero las 2000 iteraciones mi algoritmo termina.
    
    while (iteraciones<2000 && !calidad)
    {
        actualizarCentroides(dataset,centroides,asignaciones);
        if (actualizarGrupos(dataset,centroides,asignaciones)   <=  0.05f)
        {
            /* code */
        }
        
    }
}