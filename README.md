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
