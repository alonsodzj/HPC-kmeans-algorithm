#include "Punto.h"
//Implementation of the Punto class template
//solo tengo un main for testing


int main(){
    std::vector<float> valores = {1.0f, 2.0f};
    Punto<2> p2 = valores; // Esto es un punto 3D con coordenadas inicializadas a 0.0f
    for(float coord : p2.getCoords()) {
        std::cout << coord << " ";
    }
    std::cout << std::endl;
}