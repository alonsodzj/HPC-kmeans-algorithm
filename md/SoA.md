El siguiente planteamiento me va a permitir comparar el tiempo que tardo con diferentes estructuras de datos
SoA significa Struct of Arrays y quiere decir que por ejemplo por cada coordenada tengo un array que tiene tantos huecos como puntos
y cada punto de este array representa dicha coordenada en el [i] punto.

**AoS** = *Array of Structs* — **SoA** = *Struct of Arrays*

Son dos formas opuestas de organizar los mismos datos en memoria.La diferencia es puramente de organización en memoria, no de lógica. Los mismos datos, dispuestos de dos maneras:

**AoS** — piensas en términos de objetos: "dame el punto 2". En memoria quedan `x0 y0 z0 | x1 y1 z1 | x2 y2 z2 ...`. Natural para programación orientada a objetos. Pero si tu kernel de física solo necesita iterar sobre `x`, la CPU carga `y` y `z` de regalo en cada cache line aunque no los uses.

**SoA** — piensas en términos de atributos: "dame todas las x". En memoria quedan `x0 x1 x2 x3 ... | y0 y1 y2 ... | z0 z1 z2 ...`. Tres arrays independientes. Cuando iteras sobre `x`, cada cache line está repleta de `x`s útiles, nada más.

La regla práctica: si tu bucle más interno accede a **un solo campo** de muchos puntos, SoA gana. Si accede a **todos los campos** de un punto a la vez (por ejemplo, calcular la norma `sqrt(x²+y²+z²)` de un único punto), AoS no pierde casi nada porque igual necesitas cargar los tres. En HPC el primer caso es el más habitual, de ahí la preferencia por SoA.


El problema de mi anterior implementación con el vector plano AoS es el orden de los datos dentro de dicho vector.
Al ejecutar el kmeans, lo que hace principalmente es calcular distancias, para esto la implementación que tengo en un vector plano AoS es buena. En cambio al ejecutar la función de las estadísticas lo óptimo es tener el tipo de implementación SoA ya que me permite operar sobre una misma columna de manera secuencial y rápida.

// Layout recomendado para tu caso:
//
//   datos:      SoA flat  → datos[col * n_filas + fila]
//   centroides: AoS       → centroides[k * n_cols + col]
//
// Los centroides son K×n_cols floats — con K=256 y 100 cols son ~100 KB,
// caben en L2. Se reusan para todos los n_filas puntos → hot in cache.