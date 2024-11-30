
# Simulación de Paginación en Memoria

Este programa simula el mecanismo de paginación utilizado por los sistemas operativos para gestionar la memoria. A través de esta simulación, se emulan procesos que solicitan memoria, el acceso a direcciones virtuales, la generación de page faults y el reemplazo de páginas siguiendo una política específica.

## Tabla de Contenidos
- Descripción General
- Implementación
    - Inicialización de la Memoria
    - Creación y Gestión de Procesos
    - Simulación de Acceso a Direcciones Virtuales
    - Política de Reemplazo de Páginas
- Instrucciones de Ejecución
    - Compilación
    - Ejecución
    - Parámetros Solicitados


## Descripción General

El programa simula un sistema de gestión de memoria que utiliza paginación para asignar memoria a procesos. Se lleva a cabo lo siguiente:

- Inicialización de la memoria física y virtual según parámetros ingresados por consola.
- Creación de procesos de tamaño aleatorio dentro de un rango especificado, cada 2 segundos.
- Asignación de páginas a los procesos, cargándolas en RAM si hay espacio libre o en SWAP si no.
- Simulación de page faults al acceder a direcciones virtuales, generando el reemplazo de páginas cuando corresponde.
- Terminación de procesos aleatorios cada 5 segundos después de los primeros 30 segundos de ejecución.
- Uso de una política de reemplazo de páginas clara (FIFO).

## Implementación
### Inicialización de la Memoria

Al iniciar el programa, se solicitan los siguientes parámetros al usuario:

- Tamaño de la memoria física en MB.
- Tamaño de cada página en KB.

El tamaño de la memoria virtual se genera aleatoriamente, siendo entre 1.5 y 4.5 veces el tamaño de la memoria física.

La memoria física y virtual se dividen en páginas según el tamaño de página especificado. Se calcula el número total de páginas físicas y virtuales disponibles.
### Creación y Gestión de Procesos

- Creación de Procesos:
    - Cada 2 segundos, se crea un nuevo proceso.
    - El tamaño del proceso se elige aleatoriamente dentro de un rango especificado por consola.
    - Cada proceso se divide en páginas según el tamaño de página.
    - Las páginas del proceso se cargan en RAM siempre que haya espacio disponible.
    - Si la RAM se llena, las páginas restantes se cargan en la memoria swap.

- Terminación de Procesos:
    - Después de los primeros 30 segundos de ejecución, cada 5 segundos, se termina un proceso aleatorio.
    - Al terminar un proceso, se liberan sus páginas tanto de la RAM como de la memoria swap.
    - Se actualizan las estructuras de datos para reflejar la eliminación del proceso y sus páginas.

### Simulación de Acceso a Direcciones Virtuales

- Después de los primeros 30 segundos de ejecución, cada 5 segundos, se simula el acceso a una dirección virtual aleatoria.
- Se selecciona un proceso aleatorio y se genera una dirección virtual correspondiente a una de sus páginas.
- El programa determina si la página correspondiente está en RAM o en swap.
    - Si la página está en RAM: 
        - Se indica que la página ha sido encontrada y el acceso es exitoso.
    - Si la página no está en RAM (page fault): 
        - Se genera un page fault.
        - Se sigue la política de reemplazo de páginas para liberar espacio en RAM si es necesario.
        - La página requerida se carga en RAM desde swap.
        - Se actualizan las estructuras de datos para reflejar los cambios.

### Política de Reemplazo de Páginas

Se utiliza la política FIFO (First-In, First-Out) para el reemplazo de páginas en RAM.

- Funcionamiento de FIFO:
    - Cuando la RAM está llena y se necesita cargar una nueva página debido a un page fault:
        - Se identifica la página que ha estado más tiempo en RAM (la que ingresó primero).
        - Esta página se mueve a la memoria swap si hay espacio disponible.
        - Si la swap está llena, el programa finaliza indicando que no hay memoria disponible.
        Carga de la nueva página:
            La página que causó el page fault se carga en RAM.
            Se actualizan las estructuras de datos para reflejar el cambio.

## Instrucciones de Ejecución

- Compilar el programa:
``` bash
    g++ -std=c++11 -o simulacion simulacion.cpp
```

- Ejecutar el programa:
``` bash
    ./simulacion
```
