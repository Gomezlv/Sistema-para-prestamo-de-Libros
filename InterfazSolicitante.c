/*********************************************************************
* PONTIFICIA UNIVERSIDAD JAVERIANA
* AUTORES: Viviana Gómez, Roberth Méndez, Luz Adriana Salazar y Guden Silva
* FECHA: 26 de mayo de 2025
* SISTEMAS OPERATIVOS
* PROYECTO FINAL
* DESCRIPCIÓN: Interfaz con las implementaciones de las funciones que utiliza Solicitante.c
 *********************************************************************/
#include "InterfazSolicitante.h"

/*
 * Función para mostrar menu al usuario y capturar la opcion
 */
int menu(){

    int op;
    printf("\n===== MENÚ =====\n");
    printf("1. Ingresar una operación\n");
    printf("2. Salir\n");
    printf("==================\n");

    printf("Seleccione una opción: ");
    scanf("%d", &op);

    return op;

}

/*
* Función para enviar los datos de una solicitud al receptor
*/
void enviarSolicitud(int idPipeSolicitudes, char* pipeRespuestas){

    Solicitud solicitud; //Nueva solicitud

    char op; //Captura el codigo de operación
    do {

        printf("Ingrese operación (D: Devolver, R: Renovar, P: Prestar): ");
        scanf(" %c", &op);

        if(op != 'D' && op != 'R' && op != 'P')
            printf("Operación no válida. Intente nuevamente.\n");
            
    } while (op != 'D' && op != 'R' && op != 'P');
                    
    solicitud.operacion = op; //Captura la operación

    printf("Ingrese el nombre del libro: ");
    scanf(" %[^\n]", solicitud.libro); //Captura nombre de libro

    printf("Ingrese ISBN: ");
    scanf("%d", &solicitud.isbn); //Captura ISBN
                    
    strcpy(solicitud.pipe, pipeRespuestas); //Guarda nombre del pipe por el que recibe la respuesta

    escribirNamedPipe(idPipeSolicitudes, solicitud); //Envia solicitud al named pipe servidor

}

/*
* Función que finaliza la ejecución del cliente y notifica al receptor
*/
void salir(int idPipeSolicitudes, int idPipeRespuestas, char* pipeRespuestas){

    Solicitud fin = {0}; //Nueva solicitud
    fin.operacion = 'Q'; //Envía señal de salida
    strcpy(fin.libro, "Salir"); //Mensaje de salida
    strcpy(fin.pipe, pipeRespuestas);
    escribirNamedPipe(idPipeSolicitudes, fin); //Envia solicitud al named pipe servidor

    close(idPipeSolicitudes); //Cierra el named pipe
    close(idPipeRespuestas);
    unlink(pipeRespuestas);

    exit(0);
}

/*
 * Función que escribe una estructura solicitud en un named pipe
 */
void escribirNamedPipe(int idPipeSolicitudes, Solicitud solicitud){

    if (write(idPipeSolicitudes, &solicitud, sizeof(Solicitud))== -1)
        if (errno == EPIPE)
            printf("El receptor ya no está disponible. Terminando.\n");
        else
            perror("write");

    sleep(1); //Espera un segundo para que el receptor procese la solicitud

}


/*
* Función para el hilo lector que lee respuestas por named pipe
*/
void *leer(void* arg){

    int idPipeRespuestas = *(int *)arg;
    char mensaje[MAX_LINE];

    while (read(idPipeRespuestas, mensaje, sizeof(mensaje)) > 0)
        printf("Mensaje recibido: %s\n", mensaje);
    
    return NULL;

}

