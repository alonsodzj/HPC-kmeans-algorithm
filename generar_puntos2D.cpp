//esto me crea un conjuto de puntos 2D.

#include <vector>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <time.h>
#include <iomanip> // para std::setprecision

#define PI 3.141582f //constante PI

struct point2D //el punto tiene dos coordenadas x e y.
{
    float x;
    float y;
};

point2D getRandomPoint(float x0, float y0,float maxRadius, float minRadius = 0.0f )
//genera un punto aleatorio dentro de un anillo con centro en (x0,y0) y radio máximo maxRadius y mínimo minRadius
{
    point2D p;//creo el punto
    float r = minRadius + (maxRadius-minRadius) * (float)rand() / RAND_MAX; //radio aleatorio
    float alpha = 2.0f*PI* (float)rand() / RAND_MAX; //ángulo aleatorio entre 0 y 2PI, esto me da la dirección en la que se encuentra el punto respecto al centro.
    p.x = x0 + r * cos(alpha); //coseno para hallar la coordenada x
    p.y = y0 + r * sin(alpha); //seno para hallar la coordenada y
    return p; //devuelvo el punto generado
};

int main()
/*
función principal para la generación de puntos 2D, esta clase está limitada por lo que cuando terminemos el algorimto
tendremos que modificar el código para que se adapte a diferentes puntos, por ejemplo, puntos 3D, 4D, etc. Para esto se van a usar plantillas.
*/
{
    srand(time(NULL));              //semilla para la generación de números aleatorios.
    int nClusters = 1;              //creo una variable con el número de clusters que quiero generar
    int nPointsPerCluster = 20;     //creo una variable local con el número de puntos que quiero generar por cada cluster.

    std::vector<point2D> data;      //creo un vector de puntos2D para almacenar los puntos generados.
    
    //voy a dejar comentada esta parte del código porque voy a hacer debug con datos con max 1
    /*
        for (int i = 0; i < nClusters; i++)//por cada número de clusters
        {
            point2D centroid = getRandomPoint(0.0f, 0.0f, 20.0, 0.0);           //genera un centro a partir del cual los puntos se van a generar
            for (int j = 0; j < nPointsPerCluster; j++)                         //por cada número de puntos por cluster
                data.push_back(getRandomPoint(centroid.x,centroid.y, 10.0f));    //inserto dentro de mi vector de puntos un nuevo punto generado aleatoriamente a partir del centro
        }
    */
   
    for (int i = 0; i < nClusters; i++)//por cada número de clusters
    {
        point2D centroid = {0.0,0.0};   //voy a hacer que 0 sea mi centroide
        for (int j = 0; j < nPointsPerCluster; j++)                         //por cada número de puntos por cluster
            data.push_back(getRandomPoint(centroid.x,centroid.y, 1.0f));    //inserto dentro de mi vector de puntos un nuevo punto generado aleatoriamente a partir del centro
    }


    FILE* resultsFile;                              //puntero a un archivo para guardar los resultados
    resultsFile = fopen("salida", "wb");            //abro el archivo en modo escritura binaria, si el archivo no existe se crea y si existe se sobrescribe.
    int nFilas = nClusters * nPointsPerCluster;     //número de filas que tendré que leer, habrá una por cada punto generado.
    int nCol = 2;                                   //número de columnas que tendré que leer, habrá una por cada dimensión.
    fwrite(&nFilas, sizeof(int), 1, resultsFile);   //escribo el número de filas en el archivo, esto me ayuda para luego saber cuantas leer
    fwrite(&nCol, sizeof(int), 1, resultsFile);     //escribo el número de columnas en el archivo, esto me ayuda para luego saber cuantas leer
    fwrite(data.data(), sizeof(float), data.size()*nCol, resultsFile);  //escribo los datos de los puntos en el archivo, data.data() me devuelve un puntero al primer elemento del vector, sizeof(float) es el tamaño de cada elemento, data.size()*nCol es el número total de elementos que voy a escribir (número de puntos por número de dimensiones).
    fclose(resultsFile);                            //cierre de archivo, imprescindible para que se guarden los datos correctamente.

    for (int i = 0; i < data.size(); i++)           //imprimo por pantalla los puntos generados, esto es solo para debug, tendré que modificarlo.
    std::cout << std::fixed << std::setprecision(9) //pongo la precisión a 9 decimales que es lo que me da el formato float.
              << data[i].x << "\t"
              << data[i].y << "\n";
}