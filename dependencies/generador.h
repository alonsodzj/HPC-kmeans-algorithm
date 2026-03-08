#pragma once

#include <vector>
#include <cmath>
#include <random>


std::vector<float> getRandomPoint(const std::vector<float>& center, float maxRadius)
{
    int n = center.size();
    std::vector<float> point(n);                                            //creo un vector de n coordenadas
    static std::random_device rd;                                           // Semilla
    static std::mt19937 gen(rd());                                          // Generador Mersenne Twister
    std::uniform_real_distribution<float> dist(-maxRadius, maxRadius);
    for(int i = 0; i < n; i++){
            float disp = dist(gen);  //variable de desplazamiento con maxradius
        point[i] = center[i] + disp;            //aplico a cada coordenada de mi centro un desplazamiento random con maxRadius
    }
    return point;
}

