//clase que genera puntos aletorios 
#pragma once

#include "dependencies/generador.h"
#include <iostream>
#include <time.h>
#include <iomanip>                      // para std::setprecision (usado para imprimir por consola)

#define MAX_RADIUS          20.0f       //radio máximo para la generación de puntos

int main()
{
    int numeroCoordenadas, numeroClusteres, puntosCluster;
    std::cout << "Introduce el número de coordenadas: ";
    std::cin >> numeroCoordenadas;;
    std::cout << "Introduce el número de puntos: ";
    std::cin >> puntosCluster;

    srand(time(NULL));                         //semilla para la generación de números aleatorios.ss
    std::vector<std::vector<float>> data;      //creo un vector de vectores float para almacenar los puntos.
    

    //bucle para la generación de puntos a partir de los datos introducidos.
    std::vector<float> centro(numeroCoordenadas, 0.0f); //creo un centro con todas sus coordenadas en 0
    for (int j = 0; j < puntosCluster; j++)                             //por cada número de puntos por cluster
    data.push_back(generador::getRandomPoint(centro, MAX_RADIUS));    //inserto en el vector un punto aleatorio a partir del centro.

    //escritura en archivo para vectores.
    FILE* resultsFile = fopen("data/salida.bin", "wb");

    //compruebo que se haya abierto correctamente
    if (resultsFile != NULL) {

        int nFilas = puntosCluster;                     //el numero de filas es el número de puntos dentro del vector
        int nCol = numeroCoordenadas;                   //el número de columnas es una constante pero lo pongo por legibilidad

        fwrite(&nFilas, sizeof(int), 1, resultsFile);   //escribo en el archivo el número de filas
        fwrite(&nCol, sizeof(int), 1, resultsFile);     //escribo en el archivo el número de columnas

        //recorremos cada punto
        std::cout << std::fixed << std::setprecision(9);
        for (int i = 0; i < data.size(); i++) {
            fwrite(data[i].data(), sizeof(float), nCol, resultsFile);
            for (int j = 0; j < numeroCoordenadas; j++) {
                std::cout << data[i][j];
                if (j < numeroCoordenadas - 1) {
                    std::cout << "\t";
                }
            }
            std::cout << "\n";
        }
        fclose(resultsFile);
        std::cout << "Archivo guardado correctamente." << std::endl;    //esto es para depurar
    }
}
