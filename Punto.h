#include <vector>
#include <stdexcept>
#include <iostream>
/*
Creo una clase punto con plantillas para ser más flexible.
*/
template<int nDimensiones>
class Punto {
public:
    std::vector<float> coords;

    Punto() : coords(nDimensiones) {}

    Punto(const std::vector<float>& valores) : coords(valores.begin(), valores.end()) {
        if(valores.size() != nDimensiones) {
            throw std::invalid_argument("El número de valores no coincide con el número de dimensiones.");
        }
    }
};

