# Sistema para el Préstamo de Libros

## Descripción General

Este proyecto implementa un sistema de préstamo de libros utilizando conceptos fundamentales de **procesos**, **hilos** y **comunicación entre procesos (IPC)** a través de **pipes**, siguiendo el enunciado propuesto para la asignatura *Sistemas Operativos SIU4085*.

El sistema está dividido en dos componentes principales:

- **Procesos Solicitantes (PS):** ejecutan operaciones de préstamo, devolución y renovación de libros.
- **Receptor de Peticiones (RP):** recibe y gestiona las solicitudes, actualizando una base de datos local representada por un archivo de texto.

---

## Estructura del Proyecto

- `solicitante.c`: Código fuente del proceso solicitante.
- `receptor.c`: Código fuente del proceso receptor.
- `bd.txt`: Archivo que representa la base de datos de libros de la biblioteca.
- `makefile`: Archivo para compilar los ejecutables del sistema.
- `entrada.txt`: Archivo de entrada para el PS con solicitudes predefinidas.
- `README.md`: Este archivo.
- Otros archivos complementarios necesarios para la ejecución y pruebas del sistema.

---

## Compilación

Utiliza el siguiente comando para compilar el proyecto:

```bash
make
```

Esto generará los ejecutables:

- `solicitante`
- `receptor`

---

## Ejecución

### Proceso Receptor (RP)

```bash
./receptor -p pipeRP -f bd.txt [-v] [-s salida.txt]
```

**Parámetros:**

- `-p pipeRP`: Nombre del pipe nominal para comunicación.
- `-f bd.txt`: Archivo con la base de datos inicial.
- `-v`: Modo verbose (opcional), muestra las solicitudes recibidas en consola.
- `-s salida.txt`: Archivo de salida con el estado final de la BD (opcional).

### Proceso Solicitante (PS)

```bash
./solicitante [-i entrada.txt] -p pipeRP
```

**Parámetros:**

- `-i entrada.txt`: Archivo de entrada con operaciones (opcional).
- `-p pipeRP`: Pipe nominal de comunicación con el RP.

---

## Formato del Archivo de Entrada (`entrada.txt`)

```
P, Cálculo Diferencial, 120
D, Hamlet, 234
R, Historia de Colombia, 223
Q, Salir, 0
```

Donde:

- `P`: Solicitar préstamo
- `D`: Devolver libro
- `R`: Renovar libro
- `Q`: Terminar ejecución

---

## Formato de la Base de Datos (`bd.txt`)

```
Cálculo Diferencial, 120, 2
1, D, 1-10-2021
2, P, 2-10-2021

Hamlet, 234, 1
1, D, 1-10-2021
```

Cada libro está representado por:
- Título, ISBN, cantidad de ejemplares
- Por cada ejemplar: número, estado (`P` o `D`) y fecha

---

## Comandos del Receptor (por consola)

- `s`: Finaliza el proceso RP y sus hilos.
- `r`: Genera reporte en pantalla con operaciones realizadas en formato:

```
P, Nombre del Libro, ISBN, Ejemplar, Fecha
D, Nombre del Libro, ISBN, Ejemplar, Fecha
R, Nombre del Libro, ISBN, Ejemplar, Fecha
```

---

## Características Técnicas

- Implementado en **C ANSI**.
- Uso de **procesos e hilos POSIX**, **pipes con nombre (FIFOs)** y sincronización tipo **Productor-Consumidor**.
- Manejo de múltiples procesos solicitantes simultáneamente.
- Soporte para entrada manual (menú interactivo) o automática (archivo).
- Reporte detallado de las operaciones ejecutadas.

---

## Autores

- Nombre 1 – email1@dominio.com
- Nombre 2 – email2@dominio.com
- Nombre 3 – email3@dominio.com

---

## Licencia

Proyecto académico desarrollado para el curso *Sistemas Operativos SIU4085* (Marzo - Mayo 2025).
