//generador de fichero de entrada sin la clase punto
#include <vector>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <time.h>
#include <iomanip> // para std::setprecision (usado para imprimir por consola)

#define PI                  3.141582f   //constante PI
#define MAX_RADIUS          20.0f       //radio máximo para la generación de puntos
#define MAX_DISTANCE        5.0f       //distancia máxima desde el centro para la generación de puntos


std::vector<float> getRandomPoint(const std::vector<float>& center, float maxRadius, int n)
{
    std::vector<float> point(n);
    
    float norm = 0.0f;
    if (norm == 0.0f) norm = 1.0f;
    // Generar dirección aleatoria
    for (int i = 0; i < n; ++i)
    {
        point[i] = 2.0f * ((float)rand() / RAND_MAX) - 1.0f;
        norm += point[i] * point[i];
    }

    norm = std::sqrt(norm);

    // Generar radio correctamente distribuido
    float radius = maxRadius * std::pow((float)rand() / RAND_MAX, 1.0f / n);

    // Escalar y trasladar
    for (int i = 0; i < n; ++i)
        point[i] = center[i] + (point[i] / norm) * radius;

    return point;
}

int main()
/*
función principal para la generación de puntos 2D, esta clase está limitada por lo que cuando terminemos el algorimto
tendremos que modificar el código para que se adapte a diferentes puntos, por ejemplo, puntos 3D, 4D, etc. Para esto se van a usar plantillas.
*/
{
    int numeroCoordenadas, numeroClusteres, puntosCluster;
    std::cout << "Introduce el número de coordenadas: ";
    std::cin >> numeroCoordenadas;
    std::cout << "Introduce el número de clústeres: ";
    std::cin >> numeroClusteres;
    std::cout << "Introduce el número de puntos por clúster: ";
    std::cin >> puntosCluster;

    srand(time(NULL));                         //semilla para la generación de números aleatorios.ss
    std::vector<std::vector<float>> data;      //creo un vector de vectores float para almacenar los puntos.
    
    for (int i = 0; i < numeroClusteres; i++)//por cada número de clusters
    {   
        std::vector<float> centro(numeroCoordenadas, 0.0f); //creo un centro con todas sus coordenadas en 0
        std::vector<float> point = getRandomPoint(centro, MAX_RADIUS,numeroCoordenadas); //recogo el "punto" en un determinado sitio desde 0 a max radius.
        //genera un centro a partir del cual los puntos se van a generar
        for (int j = 0; j < puntosCluster; j++)                             //por cada número de puntos por cluster
        data.push_back(getRandomPoint(point, MAX_DISTANCE,numeroCoordenadas));    //inserto en el vector un punto aleatorio a partir del centro.
    }

    // --- ESCRITURA EN ARCHIVO CORRECTA PARA VECTORES ---
    FILE* resultsFile = fopen("salida.bin", "wb");
    if (resultsFile != NULL) {
        int nFilas = numeroClusteres*puntosCluster;   //el numero de filas es el número de puntos dentro del vector
        int nCol = numeroCoordenadas;                    //el número de columnas es una constante pero lo pongo por legibilidad

        fwrite(&nFilas, sizeof(int), 1, resultsFile);
        fwrite(&nCol, sizeof(int), 1, resultsFile);

        // RECORREMOS CADA PUNTO
        std::cout << std::fixed << std::setprecision(9);
        for (int i = 0; i < data.size(); i++) {
            // Escribimos los FLOATS que están DENTRO del vector del punto
            // data[i].coords.data() nos da el puntero a los números reales
            fwrite(data[i].data(), sizeof(float), nCol, resultsFile);
            for (int j = 0; j < numeroCoordenadas; j++) {
                // Imprime la coordenada j del punto i
                std::cout << data[i][j];
                
                // Si no es la última columna, ponemos un tabulador
                if (j < numeroCoordenadas - 1) {
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