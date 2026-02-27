#include "Punto.h"
#include <vector>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <time.h>
#include <iomanip> // para std::setprecision (usado para imprimir por consola)

#define PI                  3.141582f   //constante PI
#define MAX_RADIUS          20.0f       //radio máximo para la generación de puntos
#define MAX_DISTANCE        10.0f       //distancia máxima desde el centro para la generación de puntos
#define NCLUSTERS           10           //número de clusters a generar
#define NPOINTSPERCLUSTER   200          //número de puntos por cluster a generar
#define NDIMENSIONES        4           //número de dimensiones de los puntos a generar

template<int nDimensiones>
Punto<nDimensiones> getRandomPoint(const std::vector<float>& center, float maxRadius)
//esta clase lo que hace es generar un punto aleatorio dentro de una esfera con un punto centro y un radio máximo.
{
    Punto<nDimensiones> p;
    std::vector<float>& coords = p.coords;

    for(int i = 0; i < nDimensiones; ++i)
    {
        // Cada coordenada está en [center[i]-maxRadius, center[i]+maxRadius]
        float offset = (static_cast<float>(rand()) / RAND_MAX) * 2.0f * maxRadius - maxRadius;
        coords[i] = center[i] + offset;
    }
    return p;
}

int main()
/*
función principal para la generación de puntos 2D, esta clase está limitada por lo que cuando terminemos el algorimto
tendremos que modificar el código para que se adapte a diferentes puntos, por ejemplo, puntos 3D, 4D, etc. Para esto se van a usar plantillas.
*/
{
    srand(time(NULL));                          //semilla para la generación de números aleatorios.ss
    std::vector<Punto<NDIMENSIONES>> data;      //creo un vector de puntos para almacenar los puntos generados.
    
    for (int i = 0; i < NCLUSTERS; i++)//por cada número de clusters
    {   
        std::vector<float> centro(NDIMENSIONES, 0.0f); //creo un centro con todas sus coordenadas en 0
        Punto<NDIMENSIONES> centroid = getRandomPoint<NDIMENSIONES>(centro, MAX_RADIUS); 
        //genera un centro a partir del cual los puntos se van a generar
        for (int j = 0; j < NPOINTSPERCLUSTER; j++)                         //por cada número de puntos por cluster
        data.push_back(getRandomPoint<NDIMENSIONES>(centroid.coords, MAX_RADIUS));    //inserto dentro de mi vector de puntos un nuevo punto generado aleatoriamente a partir del centro
    }

    // --- ESCRITURA EN ARCHIVO CORRECTA PARA VECTORES ---
    FILE* resultsFile = fopen("salida.bin", "wb");
    if (resultsFile != NULL) {
        int nFilas = data.size(); 
        int nCol = NDIMENSIONES;

        fwrite(&nFilas, sizeof(int), 1, resultsFile);
        fwrite(&nCol, sizeof(int), 1, resultsFile);

        // RECORREMOS CADA PUNTO
        std::cout << std::fixed << std::setprecision(9);
        for (int i = 0; i < data.size(); i++) {
            // Escribimos los FLOATS que están DENTRO del vector del punto
            // data[i].coords.data() nos da el puntero a los números reales
            fwrite(data[i].coords.data(), sizeof(float), nCol, resultsFile);
            for (int j = 0; j < NDIMENSIONES; j++) {
                // Imprime la coordenada j del punto i
                std::cout << data[i].coords[j];
                
                // Si no es la última columna, ponemos un tabulador
                if (j < NDIMENSIONES - 1) {
                    std::cout << "\t";
                }
            }
            // Al terminar todas las dimensiones del punto, salto de línea
            std::cout << "\n";
        }

        fclose(resultsFile);
        std::cout << "Archivo guardado correctamente." << std::endl;
    }
}