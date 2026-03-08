#pragma once
#include <vector>
#include <cstdio>

void guardarCentroides(const std::vector<float>& centroides, int dimensiones)
{
    FILE* f = fopen("data/centroides.bin", "wb");

    if (!f) {
        printf("Error abriendo centroides.bin\n");
        return;
    }

    int nCentroides = centroides.size() / dimensiones;
    int nDim = dimensiones;

    fwrite(&nCentroides, sizeof(int), 1, f);
    fwrite(&nDim, sizeof(int), 1, f);

    fwrite(centroides.data(), sizeof(float), centroides.size(), f);

    fclose(f);
}