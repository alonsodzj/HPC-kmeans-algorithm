#pragma once

struct Dataset {
    std::vector<float> data;
    int numPuntos;
    int numCoords;
    // Constructor normal que copia
    Dataset(const std::vector<float>& datos, int puntos, int coords) : data(datos), numPuntos(puntos), numCoords(coords) {}
    // Constructor por defecto
    Dataset() : numPuntos(0), numCoords(0) {}
    // Constructor que mueve el vector (para eficiencia)
    Dataset(std::vector<float>&& datos, int puntos, int coords) : data(std::move(datos)), numPuntos(puntos), numCoords(coords) {}
};