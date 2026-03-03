tengo que calcuar esto de cad grupo media, mínimo, máximo y varianza.
Tengo diferentes clústeres que gestionan 1 o más grupos.

# Guía de Práctica: Algoritmo K-medias y Análisis de Datos

Algoritmo **k-medias**, una técnica de aprendizaje no supervisado diseñada para **agrupar conjuntos de datos en K grupos distintos** basándose en sus características y proximidad geométrica.

## 1. Objetivos del Algoritmo

* Agrupar datos con características similares en clústeres definidos.
* Minimizar la variancia intra-grupo (distancias cuadradas entre los puntos y su centroide).
* **Descubrir estructuras ocultas** o patrones no evidentes en grandes volúmenes de datos.

## 2. Definiciones Clave

| Término       | Descripción                                           |
| ---           | ---                                                   |
| **K**         | Número predefinido de grupos o clústeres a crear.     |
| **Centroide** | El centro geométrico de un grupo; el "punto promedio".|
| **Dato**      | Punto individual en un espacio de N dimensiones.      |

> **Nota sobre el centroide:** > Matemáticamente, el centroide se calcula como la media aritmética de todas las dimensiones de los puntos que pertenecen a dicho grupo.
---

## 3. Proceso del Algoritmo K-medias

### A. Inicialización
El algoritmo comienza seleccionando **K puntos al azar** dentro del espacio de datos para que actúen como los centros de grupo iniciales o centroides.

### B. Proceso Iterativo
El núcleo del algoritmo se basa en la repetición de dos pasos fundamentales:
1. **Asignación de Grupos:** Cada dato del conjunto se analiza y se asigna al **centroide más cercano**, formando así los clústeres temporales.
2. **Recálculo de Centroides:** Una vez asignados todos los puntos, se calcula la nueva posición de cada centroide moviéndolo al **centro promedio** de los puntos que tiene asignados actualmente.

### C. Criterio de Finalización (Estabilización)
Este proceso de "asignación y movimiento" se repite de forma cíclica hasta que se alcanza la **convergencia**. Esto ocurre cuando:
* Los centroides ya no cambian de posición.
* Los puntos ya no cambian de grupo entre iteraciones.
---

## 4. Aplicaciones Principales
El uso de K-medias es fundamental en diversas áreas científicas y empresariales, destacando principalmente por su capacidad para **descubrir estructuras ocultas** en los datos sin necesidad de etiquetas previas.
---


# Problemas
- Hay que elegir K manualmente.
- Puede converger a un mínimo local.
- Sensible a la inicialización.
- Funciona mejor con clusters esféricos.


[text](https://youtu.be/2kfY0R34Dy0)