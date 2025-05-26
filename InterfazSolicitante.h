/*********************************************************************
* PONTIFICIA UNIVERSIDAD JAVERIANA
* AUTORES: Viviana Gómez, Roberth Méndez, Luz Adriana Salazar y Guden Silva
* FECHA: 26 de mayo de 2025
* SISTEMAS OPERATIVOS
* PROYECTO FINAL
* DESCRIPCIÓN: Archivo con las firmas de las funciones que utiliza Solicitante.c
 *********************************************************************/
#ifndef RECEPTOR_H
#define RECEPTOR_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>

#define MAX_LINE 256

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

int menu(); //Función para mostrar menu al usuario y capturar la opcion
void enviarSolicitud(int idPipeSolicitudes, char* pipeRespuestas); //Función para enviar los datos de una solicitud al receptor
void salir(int idPipeSolicitudes, int idPipeRespuestas, char* pipeRespuestas); //Función que finaliza la ejecución del cliente y notifica al receptor
void escribirNamedPipe(int idPipeSolicitudes, Solicitud soli); //Escribe una estructura Solicitud en un named pipe
void *leer(void* arg); //Función para el hilo lector que lee respuestas por el named pipe

#endif