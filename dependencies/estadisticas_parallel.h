//--MISMO ARCHIVO PERO CON LAS FUNCIONES PARALELAS--

#pragma once

#include "dataset.h"
#include <vector>
#include <iostream>
#include <iomanip>
#include <limits>
#include <omp.h>

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
    float varianza  = 0;
    int contador    = 0;
}__attribute__((packed));

//esta función simplemente llama a la función calcularEstadísticas por cada columna de cada grupo.

//calcula todas las estadísticas en una llamada, además está paralelizda.
void calcularEstadisticas(const Dataset& data,const std::vector<int>& asignaciones) //retorno un vector con las estadísticas (ocupa relativamente poco)
{
    double t0 = omp_get_wtime();

    //ESTO NO SE MODIFICA POR LO QUE PUEDE SER COMPARTIDO
    int const numPuntos = data.numPuntos;
    int const numCoords = data.numCoords;

    std::vector<Stats> stats_total(NUM_CENTROIDES * numCoords); //tengo un struct global

    //PUEDO REORDENAR LOOPS PARA MEJORAR LA CACHÉ, EN LUGAR DE EMPEZAR POR PUNTOS, HAGO EL FOR DE CENTROIDES, LUEGO PUNTPOS
    #pragma omp parallel
    {
        std::vector<Stats> stats_local(NUM_CENTROIDES * numCoords); //tengo un struct por cada hilo

        #pragma omp for
        for(int i = 0; i < numPuntos; i++)
        {
            int grupo = asignaciones[i];

            for (int j = 0; j < numCoords ; j++)
            {
                //actualizo el correspondiente struct
                float x = data.data[i*numCoords+j];             //esto es el valor para la j coordenada
                Stats& stats = stats_local[grupo*numCoords+j];   //hago una referencia al strcut correspondiente dentro de mi vector de struct para modificarlo

                if (x < stats.min) stats.min = x; //asignación de min
                if (x > stats.max) stats.max = x; //asignación de max

                // Welford directamente sobre el vector
                stats.contador++;
                float delta = x - stats.media;
                stats.media += delta / stats.contador;    //actualización de media -> media_nueva = media_anterior + (dato_actual - media_anterior) / n
                float delta2 = x - stats.media;
                stats.varianza += delta * delta2;     //actualización de la varianza comparando el valor de esta antes y después de actualizar la media.
            }
        }
        //reduzco la sección crítica solo a la redución, voy a intentar hacerla paralela
        #pragma omp critical
        {
            //esto podíra paralelziarlo también ya que si mi número de centroides es muy grande hay muchas iteracioes.
            for (int k = 0; k < NUM_CENTROIDES * numCoords; k++)
            {
                Stats& total = stats_total[k];
                Stats& local  = stats_local[k];
                if (local.contador == 0) continue;  //si el struct no tiene datos salto a la siguiente iteración.
                // min / max
                if (local.min < total.min) total.min = local.min;
                if (local.max > total.max) total.max = local.max;
                // combinar Welford
                int nA = total.contador;
                int nB = local.contador;
                int n = nA + nB;
                float delta = local.media - total.media;

                if (n > 0)
                {
                    total.media += delta * nB / n;
                    total.varianza += local.varianza + delta * delta * nA * nB / n;
                    total.contador = n;
                }
            }
        }

        //Puedo paralelizar el de fuera o el de dentro no se cual es mejor por eso hago collapse para paralelizar los dos como uno.
        #pragma omp for collapse(n)
        for (int i = 0; i < NUM_CENTROIDES; i++)    //cada hilo hace un centroide
        {
            for (int j = 0; j < numCoords; j++)     //por cada coordenada del centroide
            {
                //cada hilo se encarga de realizar este cálculo para todas las coordenadas de su centroide
                Stats& stats = stats_total[i*numCoords+j];
                if (stats.contador > 1) stats.varianza = (stats.varianza / (stats.contador - 1));
                else stats.varianza = 0.0f;
            }
        }
        //esto solo lo ejecuta un hilo
        #pragma omp single
        {
            /*
                for (int i = 0; i < NUM_CENTROIDES; i++)
                {
                    //imprimo todas las estadísticas
                    for (int j = 0; j < numCoords; j++)
                    {
                        //imprimo las estadísticas
                        Stats& stats = stats_total[i*numCoords+j];
                        std::cout << "Coord " << j << " -> Min: " << stats.min 
                                << " | Max:   " << stats.max 
                                << " | Media: " << stats.media 
                                << " | Var:   " << stats.varianza << "\n";
                    }
                }
            */
            double t1 = omp_get_wtime();
            double tiempo = t1 - t0;
            std::cout << std::fixed << std::setprecision(6); // 6 decimales fijos
            std::cout << "Estadísticas paralelas calculadas en: " << tiempo << " segundos\n";
        }
    }
}
