Tengo que calcuar esto de cada grupo media, mínimo, máximo y varianza.
Tengo diferentes clústeres que gestionan 1 o más grupos.

*conjunto de centroides* y *conjunto de grupos de puntos* (clusteres) que intercambian puntos entre iteraciones
puede inciciarse con valor inicial para cualquiera de los dos conjutos.
En este caso se distribuye equitativamente en orden de los índices;
    el punto 1 para el grupo 1...
    el punto 2 para el grupo 2... y asi con todos.
    En caso de que haya x puntos por grupo, los x primeros al grupo 1, los x segundos al grupo 2...

Cuando ya tengo los grupos procedo a calcular los centroides.
    El centroide se calcula como el valor medio en cada columna de los datos(puntos).

Los centroides se actualizan una vez se modifican los grupos en cada iteración.
    **vuelvo a realizar el mismo cálculo para los centroides**
    los nodos se comunican el cambio de los cengtroides para el siguiente paso
    
Cada vez que los centroides se actualizan debe calcluarse para cada punto la distancia al centroide 
y asignar el punto al grupo que tenga menor distancia al centroide

**los nodos se comunican entre sí para enviarse los puntos que les pertenecen**

# criterio de finalización
- 2000 iteraciones
- Que el número total de puntos desplazados sea menor al 5% - o que todos los nodos muevan menos de 5% dentro suya

[text](https://youtu.be/2kfY0R34Dy0)


## Una vez paralelizado:
🚀 Optimizaciones avanzadas a considerar
1. Lloyd's acceleración (Triangle Inequality)
En lugar de calcular distancia a todos los centroides, usa la desigualdad triangular para podar cálculos.
2. Mini-batch K-Means
En cada iteración, procesa solo una muestra aleatoria de puntos. Converge más rápido.
3. Computación asíncrona
Solapar comunicación MPI con cómputo usando MPI_Isend/Irecv + MPI_Wait.
