#pragma once

#include "dataset.h"
#include <vector>
#include <iostream>
#include <iomanip>
#include <limits>

#define NUM_CENTROIDES 5
#define NUM_STATS 4

//--ESTADÍSTICAS DE UNA COLUMNA--
/*
    La clave del máximo rendimiento aquí es procesar cada punto una sola vez, 
    identificar a qué grupo pertenece mediante el vector asignaciones, 
    y actualizar las estadísticas de todas sus coordenadas en ese mismo instante.
*/
//creo un struct para almacenar las estadísticas.
struct Stats{
    float min       = std::numeric_limits<float>::max();        //para no inicializar 0
    float max       = std::numeric_limits<float>::lowest();     //para no inicializar a 0
    float media     = 0;
    float M2        = 0;
    int contador    = 0;
};

//esta función simplemente llama a la función calcularEstadísticas por cada columna de cada grupo.
//¿puedo tener una función que calcule tódas las estadísticas con una sola llamada...?
void calcularEstadisticas(const Dataset& data,const std::vector<int>& asignaciones) //retorno un vector con las estadísticas (ocupa relativamente poco)
{
    clock_t time0 = clock();    //para medir cuanto tarda
    
    //trabajamos con variables para iterar sobre variables en registros
    int numPuntos = data.numPuntos;
    int numCoords = data.numCoords;

    //aquí realmente me da igual tener un puntero de punteros porque realmente no me impacta tanto en el rendimiento de todas formas puedo probar a hacerlo lineal.
    std::vector<Stats> todasStats(NUM_CENTROIDES*numCoords); //tengo un struct por cada coordenada de cada grupo.
    
    //ahora hago la pasada general para calcular todas las estadísticas.

    for(int i = 0; i < numPuntos; i++)
    //por cada punto
    {
        int grupo = asignaciones[i];    //guardo el índice del grupo al que pertenece el punto para saber que struct modificar para que coordenada.

        for (int j = 0; j < numCoords ; j++)
        //por cada coordenada de dicho punto
        {
            //actualizo el correspondiente struct
            float x = data.data[i*numCoords+j]; //esto es el valor para la j coordenada
            Stats& stats = todasStats[grupo*numCoords+j];   //hago una referencia al strcut correspondiente dentro de mi vector de struct para modificarlo

            if (x < stats.min) stats.min = x; //asignación de min
            if (x > stats.max) stats.max = x; //asignación de max

            // Welford directamente sobre el vector
            stats.contador++;
            float delta = x - stats.media;
            stats.media += delta / stats.contador;    //actualización de media -> media_nueva = media_anterior + (dato_actual - media_anterior) / n
            float delta2 = x - stats.media;
            stats.M2 += delta * delta2;     //actualización de la varianza comparando el valor de esta antes y después de actualizar la media.
        }
    }
    //teoricamente aquí todavía no estarían bien calculados algunas de las estadísticas como por ejemplo la varianza
    std::cout<< std::fixed << std::setprecision(9);
    for (int i = 0; i < NUM_CENTROIDES; i++)
    //por cada grupo
    {
        std::cout << "grupo: " << i << "\n";
        for (int j = 0; j < numCoords; j++)
        //por cada coordenada del grupo
        {
            //imprimo las estadísticas
            Stats& stats = todasStats[i*numCoords+j];
            float varianza;

            if (stats.contador > 1) varianza = (stats.M2 / (stats.contador - 1));
            else varianza = 0.0f;

            std::cout << "Coord " << j << " -> Min: " << stats.min 
                      << " | Max:   " << stats.max 
                      << " | Media: " << stats.media 
                      << " | Var:   " << varianza << "\n";
        }
    }
    clock_t time1 = clock();

    double tiempo = double(time1 - time0) / CLOCKS_PER_SEC;

    std::cout << std::fixed << std::setprecision(6); // 6 decimales fijos
    std::cout << "Estadísticas calculadas en: " << tiempo << " segundos\n";
}

//esta versión es la de calcular todas las estadísticas de seguido con el algoritmo de welford (una sola pasada) para los datos estructurados en un solo vector
std::vector<float> calcStatsCol(const std::vector<float>& columna)
{
    int numPuntos = columna.size(); //el número de puntos
    if (numPuntos == 0) return std::vector<float>(4, 0.0f); //si no tengo puntos retorno 0 para todas (ojo el main no debería permitir ni kmedias ni esto con 0 puntos)
    
    std::vector<float> stats(NUM_STATS);

    stats[0] = columna[0];  //  min
    stats[1] = columna[0];  //  max
    stats[2] = 0;           //  media
    stats[3] = 0;           //  acumulador de varianza (M2)

    for (int i = 0; i < numPuntos; i++)
    {
        float x = columna[i];
        if (x < stats[0]) stats[0] = x; //asignación de min
        if (x > stats[1]) stats[1] = x; //asignación de max

        // Welford directamente sobre el vector
        float delta = x - stats[2];
        stats[2] += delta / (i + 1);    //actualización de media -> media_nueva = media_anterior + (dato_actual - media_anterior) / n
        float delta2 = x - stats[2];
        stats[3] += delta * delta2;     //actualización de la varianza comparando el valor de esta antes y después de actualizar la media.
    }
    stats[3] /= (numPuntos - 1);    //final de varianza

    return stats;
}