#pragma once

#include <vector>
#include <iostream>
#include <cmath>
#include <omp.h>
#include <time.h>
#include <iomanip>

/**
 * * @param datos Vector plano con los datos del dataset.
 * @param n_filas Número de puntos (muestras).
 * @param n_columnas Número de dimensiones de cada punto.
 * @param centroides Vector con los centroides iniciales (se actualizan in-place).
 * @param asignaciones Vector donde se guarda el clúster de cada punto.
 */
void kmeans(const std::vector<float>& datos, size_t n_filas, size_t n_columnas, std::vector<float>& centroides, std::vector<int>& asignaciones) {
    
    //el número de centroides es el tamaño del vector de centroides (puntos) entre las coordenadas de los mismos
    int num_centroides = centroides.size() / n_columnas;

    bool calidad = false;   //creo una flag calidad que me va a servir para comprobar si se superó o no el umbral del algoritmo
    int iteraciones = 0;    //creo un contador de iteraciones para llevar la cuenta de las veces que se ejecuta

    //inicializo el vector de asignaciones.
    for (size_t i = 0; i < n_filas; ++i) 
    //por cada punto, entonces:
    {
        asignaciones[i] = i % num_centroides; //aquí relleno el vector con round robin (grupos)
    }

    //aquí es donde se ejecuta el algoritmo
    while (iteraciones < 2000 && !calidad) 
    //mientras que no se supere el número de iteraciones o se llegue al umbral de calidad, entonces:
    {
        //actualizo los centroides (recalculo la media de todos los)

        std::vector<float> suma(centroides.size(), 0.0f);   
        //creo un vector de floats de tamaño numcentroides que va a almacenar la suma acumulada de los datos de cada coordenada antes de hacer la media

        std::vector<int> conteo(num_centroides, 0); //esto lleva la cuenta de los puntos que tiene cada centroide

        //recorro de una pasada todo mi vector de puntos para calcular el punto al que derivan los centroides
        for (size_t i = 0; i < n_filas; ++i)
        //por cada punto
        {
            int c_idx = asignaciones[i];    //es el índice del grupo al que pertenece
            conteo[c_idx]++;                //indico que el grupo tiene asignado un punto más
            for (size_t j = 0; j < n_columnas; ++j) 
            //por cada columna de este punto
            {
                //sumo el valor de dicha coordenada al centroide (grupo) que pertenece
                suma[c_idx * n_columnas + j] += datos[i * n_columnas + j];
            }
        }

        //por cada centroide tengo que hacer la división de la suma entre el número de puntos que le pertenezcan
        for (int i = 0; i < num_centroides; ++i) 
        //por cada centroide 
        {
            if (conteo[i] > 0) 
            //si al menos tiene un punto asignado, calculo su suma acumulada
            {
                for (size_t j = 0; j < n_columnas; ++j) 
                //por cada columna del centroide hago la división para calcular el nuevo centroide
                {
                    centroides[i * n_columnas + j] = suma[i * n_columnas + j] / conteo[i];
                }
            }
        }

        //actualizo los grupos y calculo la convergencia

        int puntos_desplazados = 0; //variable para calcular el número de puntos que se desplazan de un grupo a otro

        for (size_t i = 0; i < n_filas; ++i) 
        //por cada punto
        {
            int mejor_centroide = asignaciones[i];  //el mejor centroide empieza siendo el actual
            float menor_distancia = -1.0f;  //ponemos una distancia negativa para calcular correctamente

            for (int j = 0; j < num_centroides; ++j) {

                float distancia_acumulada = 0.0f;   //la distancia acumulada es la suma de las distancias en cada coordenada
                for (size_t d = 0; d < n_columnas; ++d) 
                //por cada coordenada
                {
                    float diferencia = datos[i * n_columnas + d] - centroides[j * n_columnas + d];
                    distancia_acumulada += diferencia * diferencia; //para evitar el uso de pow hago x*x
                }

                //si la distancia acumulada es menor que la distancia actual entonces tengo que sustituir la distancia actual por la acumulada.
                if (j == 0 || distancia_acumulada < menor_distancia) {
                    //actualizo tanto la mejor distancia como el mejor centroide (grupo al aque ahora pertenece)
                    menor_distancia = distancia_acumulada;
                    mejor_centroide = j;
                }
            }

            if (asignaciones[i] != mejor_centroide) 
            //si se ha cambiado el mejor centroide cuento un desplazamiento y actualizo el grupo/centroide al que pertenece
            {
                puntos_desplazados++;
                asignaciones[i] = mejor_centroide;
            }
        }

        //compruebo si he superado el umbral
        float tasa_cambio = static_cast<float>(puntos_desplazados) / n_filas;
        if (tasa_cambio <= 0.001f) {
            calidad = true;
        }
        iteraciones++;
    }
}