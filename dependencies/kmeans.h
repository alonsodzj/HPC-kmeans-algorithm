#pragma once

#include <vector>
#include <iostream>
#include <cmath>
#include <omp.h>
#include <time.h>
#include <iomanip>
#include "dataset.h"

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
    int puntosDesplazados = 0;
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
        if (asignaciones[i] != mejorCentroide)
        {
            puntosDesplazados++;
            asignaciones[i] = mejorCentroide;
        }
    }
    return float(puntosDesplazados)/dataset.numPuntos;
}

//--FUNCIÓN KMEANS SECUENCIAL--
void kmeans(const Dataset& dataset, std::vector<float>& centroides, std::vector<int>&asignaciones)
{
    clock_t time0 = clock();

    //asigno los puntos a un determinado grupo antes de calcular los centroides
    for(int i = 0; i < asignaciones.size(); i++){
        asignaciones[i] = i % (centroides.size() / dataset.numCoords);
    }

    bool calidad = false;   
    int iteraciones = 0;

    while (iteraciones < 2000 && !calidad)
    {
        actualizarCentroides(dataset, centroides, asignaciones);

        if (actualizarGrupos(dataset, centroides, asignaciones) <= 0.0001f)
        {
            calidad = true;
        }
        iteraciones++;
    }

    clock_t time1 = clock();

    double tiempo = double(time1 - time0) / CLOCKS_PER_SEC;

    std::cout << std::fixed << std::setprecision(6); // 6 decimales fijos
    std::cout << "Algoritmo finalizado en tiempo de ejecución: "
              << tiempo << " segundos\n";
}