**omp_set_num_threads(int)** define el número de hilos
**omp_get_num_threads()** devuelve el número de hilos activos
**omp_get_max_threads()** devuelve el máximo número de hilos disponibles
**omp_get_thread_num()** devuelve el ID del hilo actual
**omp_get_num_procs()** devuelve el número de CPUs disponibles
**omp_in_parallel()** comprueba si se está dentro de una región paralela

---

**omp_set_dynamic(int)** activa o desactiva el ajuste dinámico del número de hilos
**omp_get_dynamic()** devuelve si el ajuste dinámico está activo

---

**omp_set_nested(int)** activa o desactiva el paralelismo anidado
**omp_get_nested()** devuelve si el paralelismo anidado está activo

---

**omp_set_schedule(omp_sched_t, int)** define el tipo de scheduling de los bucles
**omp_get_schedule(omp_sched_t*, int*)** obtiene el scheduling actual

---

**omp_get_wtime()** devuelve el tiempo actual en segundos
**omp_get_wtick()** devuelve la resolución del reloj de tiempo

---

**omp_init_lock(omp_lock_t*)** inicializa un lock
**omp_destroy_lock(omp_lock_t*)** destruye un lock
**omp_set_lock(omp_lock_t*)** bloquea el lock
**omp_unset_lock(omp_lock_t*)** desbloquea el lock
**omp_test_lock(omp_lock_t*)** intenta bloquear el lock sin esperar

---

**omp_init_nest_lock(omp_nest_lock_t*)** inicializa un lock anidado
**omp_destroy_nest_lock(omp_nest_lock_t*)** destruye un lock anidado
**omp_set_nest_lock(omp_nest_lock_t*)** bloquea un lock anidado
**omp_unset_nest_lock(omp_nest_lock_t*)** desbloquea un lock anidado
**omp_test_nest_lock(omp_nest_lock_t*)** intenta bloquear un lock anidado

---

**omp_get_level()** devuelve el nivel actual de paralelismo anidado
**omp_get_active_level()** devuelve el número de niveles paralelos activos
**omp_get_ancestor_thread_num(int)** devuelve el ID del hilo ancestro
**omp_get_team_size(int)** devuelve el tamaño del equipo en un nivel dado

---

**omp_get_proc_bind()** devuelve la política de afinidad de los hilos
**omp_get_num_places()** devuelve el número de lugares de ejecución disponibles
**omp_get_place_num()** devuelve el lugar de ejecución actual
**omp_get_place_num_procs(int)** devuelve cuántos procesadores tiene un lugar
**omp_get_place_proc_ids(int, int*)** devuelve los IDs de CPU de un lugar

---

**omp_get_num_devices()** devuelve el número de dispositivos disponibles (GPUs)
**omp_get_default_device()** devuelve el dispositivo por defecto
**omp_set_default_device(int)** establece el dispositivo por defecto
**omp_get_initial_device()** devuelve el dispositivo inicial del host

---

**omp_target_alloc(size_t, int)** reserva memoria en un dispositivo
**omp_target_free(void*, int)** libera memoria en un dispositivo

---

**omp_alloc(size_t, omp_allocator_handle_t)** reserva memoria usando un allocator de OpenMP
**omp_free(void*, omp_allocator_handle_t)** libera memoria asignada con OpenMP
**omp_get_default_allocator()** devuelve el allocator por defecto
**omp_set_default_allocator(omp_allocator_handle_t)** establece el allocator por defecto

---

# Directivas OpenMP

## Paralelismo básico

**#pragma omp parallel** crea una región paralela donde varios hilos ejecutan el mismo bloque de código simultáneamente. Cada hilo ejecuta una copia del bloque. Se usa para iniciar paralelismo en el programa.

**#pragma omp parallel num_threads(n)** crea una región paralela especificando el número de hilos que deben ejecutarse.

**#pragma omp parallel if(condición)** solo crea la región paralela si la condición es verdadera; si no, se ejecuta secuencialmente.

**#pragma omp parallel default(shared|none|private)** define cómo se comportan las variables por defecto dentro de la región paralela.

## Paralelización de bucles

**#pragma omp for** distribuye las iteraciones de un bucle entre los hilos de una región paralela. Cada hilo ejecuta un subconjunto de iteraciones.

**#pragma omp parallel for** combina la creación de la región paralela y la distribución del bucle en una sola directiva.

**#pragma omp for schedule(tipo)** controla cómo se reparten las iteraciones entre los hilos.

Tipos de scheduling:

* **static** divide las iteraciones de forma fija entre hilos.
* **dynamic** reparte iteraciones a medida que los hilos terminan trabajo.
* **guided** similar a dynamic pero empieza con bloques grandes y luego pequeños.
* **auto** el compilador decide la estrategia.

**#pragma omp for collapse(n)** combina varios bucles anidados para paralelizarlos como si fueran uno solo.

**#pragma omp for nowait** evita la barrera implícita al final del bucle.

## Sincronización

**#pragma omp barrier** fuerza a que todos los hilos esperen hasta que todos alcancen ese punto antes de continuar.

**#pragma omp critical** define una sección crítica que solo puede ser ejecutada por un hilo a la vez para evitar condiciones de carrera.

**#pragma omp critical(nombre)** permite tener varias secciones críticas independientes usando nombres.

**#pragma omp atomic** garantiza que una operación simple sobre una variable compartida se ejecute de forma atómica.

Operaciones típicas:

* incremento
* decremento
* suma
* asignación

**#pragma omp flush** sincroniza las variables entre los hilos para garantizar coherencia de memoria.

## Control de ejecución de hilos

**#pragma omp single** garantiza que solo un hilo ejecuta el bloque de código mientras los demás esperan al final.

**#pragma omp single nowait** igual que single pero los otros hilos no esperan.

**#pragma omp master** solo el hilo maestro ejecuta el bloque. A diferencia de single, no hay barrera implícita.

## Trabajo dividido en secciones

**#pragma omp sections** divide el trabajo en bloques independientes que se reparten entre los hilos.

**#pragma omp section** define cada bloque de trabajo dentro de sections.

Cada sección es ejecutada por un hilo diferente.

## Tareas (task parallelism)

**#pragma omp task** crea una tarea que puede ejecutarse de forma paralela por cualquier hilo disponible.

Se usa para:

* algoritmos recursivos
* grafos
* workloads irregulares

**#pragma omp taskwait** obliga a esperar a que todas las tareas hijas terminen antes de continuar.

**#pragma omp taskgroup** agrupa tareas para sincronizarlas juntas.

**#pragma omp taskyield** permite que el hilo actual ceda la CPU a otras tareas pendientes.

## Gestión de datos

**#pragma omp private(variable)** cada hilo tiene su propia copia de la variable.

**#pragma omp firstprivate(variable)** cada hilo recibe una copia inicializada con el valor original.

**#pragma omp lastprivate(variable)** la variable mantiene el valor de la última iteración del bucle.

**#pragma omp shared(variable)** la variable es compartida por todos los hilos.

**#pragma omp reduction(operador:variable)** combina resultados parciales de cada hilo en una sola variable al final.

Operadores comunes:

* *
* *
* max
* min
* &&
* ||

Ejemplo conceptual:

cada hilo calcula su suma → al final se combinan.

## Control de memoria

**#pragma omp threadprivate(variable)** hace que una variable global tenga una copia separada para cada hilo.

**#pragma omp copyin(variable)** copia el valor inicial de una variable threadprivate al inicio de una región paralela.

## Paralelismo en dispositivos (GPU)

Desde OpenMP 4+.

**#pragma omp target** ejecuta el código en un dispositivo externo como una GPU.

**#pragma omp target data** define una región donde los datos se transfieren entre CPU y GPU.

**#pragma omp target update** actualiza datos entre host y dispositivo.

**#pragma omp target teams** crea equipos de hilos en el dispositivo.

**#pragma omp target parallel for** paraleliza bucles directamente en la GPU.

## SIMD (vectorización)

**#pragma omp simd** fuerza la vectorización del bucle usando instrucciones SIMD del procesador.

**#pragma omp declare simd** permite que una función tenga versiones vectorizadas.

## Cancelación

**#pragma omp cancel** cancela la ejecución de ciertas regiones paralelas.

Tipos:

* for
* sections
* taskgroup
* parallel

**#pragma omp cancellation point** punto donde se comprueba si se debe cancelar.

## Orden de ejecución

**#pragma omp ordered** asegura que ciertas partes del bucle se ejecuten en el orden original.

# Las directivas más importantes (las que más se usan)

En práctica, el **90% del código OpenMP usa solo estas**:

#pragma omp parallel
#pragma omp parallel for
#pragma omp for
#pragma omp critical
#pragma omp atomic
#pragma omp barrier
#pragma omp single
#pragma omp sections
#pragma omp task
#pragma omp reduction
