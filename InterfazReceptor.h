/*********************************************************************
* PONTIFICIA UNIVERSIDAD JAVERIANA
* AUTORES: Viviana Gómez, Roberth Méndez, Luz Adriana Salazar y Guden Silva
* FECHA: 26 de mayo de 2025
* SISTEMAS OPERATIVOS
* PROYECTO FINAL
* DESCRIPCIÓN: Archivo con las firmas de las funciones que utiliza Receptor.c
 *********************************************************************/
#ifndef RECEPTOR_H
#define RECEPTOR_H 
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <strings.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <time.h>
#define MAX_LINE 256
#define BUFFER_SIZE 10
/*
 * Estructura para guardar los datos de una solicitud:
 * Operación, Nombre del libro, ISBN y el nombre del
 * pipe del proceso solicitante.
 */
typedef struct {
    char operacion; // 'D' (devolver), 'R' (renovar), 'P' (prestar)
    char libro[100];
    int isbn;
    char pipe[100];
} Solicitud;

/*
 * Estructura para guardar los datos de los ejemplares
 * de cada libro.
 */
typedef struct {
    char estado;
    char nombre[100];
    int isbn;
    int ejemplar;
    struct tm fecha;
} Libro;

/*
 * Variables Globales
 */
extern int out, totalLibros, idPipeSolicitudes, in, contador, solicitantesActivos;
extern Libro bd[100], reporte[100]; // Inicializa elementos en 0
extern Solicitud buffer[BUFFER_SIZE];

extern pthread_cond_t noVacio;
extern pthread_mutex_t mutexBuffer;
extern pthread_mutex_t mutexBD;
extern pthread_cond_t noLleno;
extern pthread_mutex_t lock;

void cargarBD(char* archivoBD); //Función que lee el archivo bd y la carga en el arreglo bd
bool buscar_libro(int isbn, char* nombre);
void *auxiliar1_(void *arg);
void prestar_libro(Solicitud solicitud, char fecha_devolucion[20]);
void devolver(Solicitud soli, int fifoRespuesta); //Función que procesa la devolución con un libro previamente prestado
void renovar(Solicitud soli, int fifoRespuesta); //Función que procesa la renovación del prestamo de una libro previamente prestado.
Libro* buscarLibroPrestado(Solicitud soli); //Función que busca un libro prestado en la base de datos 
void* auxiliar2_(void* arg);
void actualizar_solicitante(Solicitud sol); //Función que actualiza el contador global de solicitantes activos
void actualizar_bd(char* archivoBD);
void escribir_reporte(const char* nombre_archivo); //Función que genera un reporte y lo escribe en un archivo de texto
void prestamo(Solicitud solicitud);
void devolverRenovar(Solicitud solicitud);

#endif