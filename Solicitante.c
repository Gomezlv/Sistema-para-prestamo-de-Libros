/*********************************************************************
* PONTIFICIA UNIVERSIDAD JAVERIANA
* AUTORES: Viviana Gómez, Roberth Méndez, Luz Adriana Salazar y Guden Silva
* FECHA: 26 de mayo de 2025
* SISTEMAS OPERATIVOS
* PROYECTO FINAL
* DESCRIPCIÓN: Código main del proceso solicitante
 *********************************************************************/
#include "InterfazSolicitante.h"

/*
* Función principal del solicitante.
*/
int main(int argc, char *argv[]) {

    char pipeSolicitudes[256]; //Ruta del named pipe para enviar solicitudes
    char *archivoEntrada = NULL; //Nombre del archivo de entrada
    int opt;

    //Verifica cantidad de argumentos
    if(argc != 5 && argc != 3){
        printf("Uso: %s [-i archivo_entrada] -p named pipe\n", argv[0]);
        exit(1);
    }

    //Parseo de argumentos -i, -p
    while ((opt = getopt(argc, argv, "i:p:")) != -1) {
        switch (opt) {
            case 'i':
                archivoEntrada = optarg; //Guarda el nombre de archivo de entrada
                break;
            case 'p':
                snprintf(pipeSolicitudes, sizeof(pipeSolicitudes), "/tmp/%s", optarg); //Concatena el nombre del pipe con /tmp/
                break;
            default:
                printf("Uso: %s [-i archivo_entrada] -p named pipe\n", argv[0]); //Imprime forma correcta de usar el comando
                exit(1);
        }
    }

    //Verificar que se definio el nombre del named pipe
    if (pipeSolicitudes == NULL) {
        printf("Debe especificar el nombre del named pipe con -p\n");
        exit(1);
    }

    //Crea el named pipe de respuestas
    char pipeRespuestas[100]; //Nombre del named pipe cliente
    pid_t pid = getpid();
    snprintf(pipeRespuestas, sizeof(pipeRespuestas), "/tmp/%d", pid); //Crea el nombre del pipe de respuestas

    //Abre named pipe del receptor en modo escritura
    int idPipeSolicitudes = open(pipeSolicitudes, O_WRONLY);
    if (idPipeSolicitudes == -1) {
        perror("open");
        exit(1);
    } else {

        Solicitud inicio = {0}; //Crea solicitud de inicio para el receptor
        inicio.operacion = 'I'; //Operación de señal de inicio
        strcpy(inicio.pipe, pipeRespuestas); //Asigna el nombre del pipe de respuestas
        escribirNamedPipe(idPipeSolicitudes, inicio); //Envia la solicitud al receptor
    }

    //Crea el pipe de respuestas y lo abre en modo lectura/escritura
    mkfifo(pipeRespuestas, 0666);
    int idPipeRespuestas = open(pipeRespuestas, O_RDWR);
    if (idPipeRespuestas == -1) {
        perror("open");
        exit(1);
    }

    //Crea hilo para leer respuestas del receptor
    pthread_t lector;
    pthread_create(&lector, NULL, leer, &idPipeRespuestas);

    //Si no se especifico archivo de entrada, entra en modo interactivo
    if(archivoEntrada == NULL) {
        while(1)
            switch(menu()) {
                case 1:
                    enviarSolicitud(idPipeSolicitudes, pipeRespuestas); //Función que envía solicitud por el pipe 
                break;

                case 2:
                    salir(idPipeSolicitudes, idPipeRespuestas, pipeRespuestas); //Función que envía señal de salida al receptor
                break;

                default:
                    printf("Opción no válida");
                break;
            }
    } else {
        //Modo lectura desde archivo
        FILE *input = stdin;
        
        if (archivoEntrada != NULL) {
            input = fopen(archivoEntrada, "r"); //Abre archivo de entrada
            if (input == NULL) {
                perror("fopen");
                exit(1);
            }
        }

        char linea[MAX_LINE]; //Buffer para la linea leida

        while (fgets(linea, MAX_LINE, input) != NULL) { //Lee linea por linea

            Solicitud solicitud;
            sscanf(linea, "%c, %[^,], %d", &solicitud.operacion, solicitud.libro, &solicitud.isbn); //Parsea la linea
            strcpy(solicitud.pipe, pipeRespuestas); //Asigna pipe del cliente
            
            escribirNamedPipe(idPipeSolicitudes, solicitud); //Envia la solicitud

        }

        if (archivoEntrada != NULL)
            fclose(input); //Cierra el archivo
        
    }

    return 0;

}

