/*********************************************************************
* PONTIFICIA UNIVERSIDAD JAVERIANA
* AUTORES: Viviana Gómez, Roberth Méndez, Luz Adriana Salazar y Guden Silva
* FECHA: 26 de mayo de 2025
* SISTEMAS OPERATIVOS
* PROYECTO FINAL
* DESCRIPCIÓN: Código main del proceso Receptor
 *********************************************************************/
#include "InterfazReceptor.h"

int main(int argc, char *argv[]) {

    char pipeSolicitudes[256], mensaje[MAX_LINE];
    char *archivoBD = NULL;
    char *reporte = NULL;
    int comando;
    bool verbose = false;

    // Parsear argumentos -p, -f,  -v, -s
    while ((comando = getopt(argc, argv, "p:f:vs:")) != -1) {
        switch (comando) {
            case 'p':
                snprintf(pipeSolicitudes, sizeof(pipeSolicitudes), "/tmp/%s", optarg); //Concatena el nombre del pipe con /tmp/ y lo guarda
                break;
            case 'f':
                archivoBD = optarg; // Guarda el nombre del archivo de la base de datos
                break;
            case 'v':
                verbose = true; // Pone verbose en true si recibe el comando -v
                break;
            case 's':
                reporte = optarg; // Guarda el nombre del archivo del reporte
                break;
            default:
                fprintf(stderr, "Uso: %s -p fifo -f archivo_bd\n", argv[0]); // Imprime forma correcta de usar los comandos
                exit(EXIT_FAILURE);
        }
    }

    //Verifica que se haya ingresado nombre del FIFO y la base de datos
    if (pipeSolicitudes == NULL || archivoBD == NULL) {
        fprintf(stderr, "Debe especificar el nombre del FIFO con -p y el archivo de BD con -f\n");
        exit(EXIT_FAILURE);
    }

    // Crear named pipe si no existe
    mkfifo(pipeSolicitudes, 0666);
    idPipeSolicitudes = open(pipeSolicitudes, O_RDWR);
    if (idPipeSolicitudes == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    
    cargarBD(archivoBD); // Cargar la base de datos

    pthread_t auxiliar1, auxiliar2; // Crea los hilos auxiliar1 y auxilar2
     
    pthread_create(&auxiliar1, NULL, auxiliar1_, NULL); //Hilo encargado de devolver y renovar bd
    pthread_create(&auxiliar2, NULL, auxiliar2_, NULL); //Hilo encargado de recibir comandos del usuario

    Solicitud solicitud; // Variable en que se guardan las solicitudes recibidas

    while (read(idPipeSolicitudes, &solicitud, sizeof(Solicitud)) > 0) {

        if(verbose && solicitud.operacion != 'S' && solicitud.operacion != 'I')
            printf("\nSolicitud recibida: %c, %s, %d\n", solicitud.operacion, solicitud.libro, solicitud.isbn); // Imprime la solicitud si el verbose esta activo

        if(solicitud.operacion == 'I') {
            // Solicitud de inicio para registrar el pipe del solicitante
            actualizar_solicitante(solicitud); // Actualiza la cantidad de solicitantes activos
            continue;
        }
        
        if(solicitud.operacion == 'Q'){
            if(verbose == true)
                printf("Solicitud de salida recibida\n"); 
            actualizar_solicitante(solicitud); // Actualiza la cantidad de solicitantes activos
        }
            
        if(solicitantesActivos == 0){
            
            contador = -1; // Indica que no hay clientes activos
            pthread_cond_signal(&noVacio); // Desbloquea el hilo auxiliar1
            solicitud.operacion = 'S'; // Indica que no hay clientes activos
            printf("\n¡¡¡¡¡¡Todos los procesos solicitantes han terminado!!!!!!\n");
            printf("Presione ENTER para salir\n");

        }

        // Procesar la solicitud
        if (solicitud.operacion == 'P'){            
            pthread_mutex_lock(&mutexBD); // Mutex que asegura que solo un hilo acceda a la base de datos
            prestamo(solicitud);
            pthread_mutex_unlock(&mutexBD); //Desbloquea el mutex de la base de datos
        }
        else if (solicitud.operacion == 'D' || solicitud.operacion == 'R'){
            devolverRenovar(solicitud);
        }   
        else if (solicitud.operacion == 'S') {

            actualizar_bd(archivoBD); // Actualiza la base de datos
            if(reporte != NULL)
                escribir_reporte(reporte); // Escribir reporte de la base de datos

            close(idPipeSolicitudes);
            break;
        }

    }

    // Esperar a que los hilos terminen
    pthread_join(auxiliar2, NULL);
    pthread_join(auxiliar1, NULL);

    unlink(pipeSolicitudes); // Eliminar archivo FIFO del sistema operativo
    return 0;

}

