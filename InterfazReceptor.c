/*********************************************************************
* PONTIFICIA UNIVERSIDAD JAVERIANA
* AUTORES: Viviana Gómez, Roberth Méndez, Luz Adriana Salazar y Guden Silva
* FECHA: 26 de mayo de 2025
* SISTEMAS OPERATIVOS
* PROYECTO FINAL
* DESCRIPCIÓN: Interfaz con las implementaciones de las funciones que utiliza Receptor.c
 *********************************************************************/
#include "InterfazReceptor.h"

/*
 * Variables Globales
 */
int out = 0, totalLibros = 0, idPipeSolicitudes, in = 0, contador = 0, solicitantesActivos = 0;
Libro bd[100], reporte[100] = {0}; // Inicializa elementos en 0
Solicitud buffer[BUFFER_SIZE];

pthread_cond_t noVacio = PTHREAD_COND_INITIALIZER; // Variable de condición para el mutex de productor-consumidor
pthread_mutex_t mutexBuffer = PTHREAD_MUTEX_INITIALIZER; // Mutex para el búfer del productor-consumidor
pthread_mutex_t mutexBD = PTHREAD_MUTEX_INITIALIZER; // Mútex para acceder a la base de datos
pthread_cond_t noLleno = PTHREAD_COND_INITIALIZER; // Variable de condición para el mutex de productor-consumidor
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * Función encargada de leer el archivo de la base de datos
 * y cargarla al arrego bd.
 */
void cargarBD(char* archivoBD){
    
    char line[MAX_LINE]; // Variable para leer cada linea de la base de datos
    char nombre[100]; // Variable para guardar el nombre del libro
    int isbn; // Variable para guardar el isbn del libro
    int copias; // Número de copias de cada libro

    FILE *archivo_bd = stdin;
    if (archivoBD != NULL) {
        archivo_bd = fopen(archivoBD, "r");
        if (archivo_bd == NULL) {
            perror("fopen");
            exit(EXIT_FAILURE);
        }
    }
    
    while (fgets(line, MAX_LINE, archivo_bd) != NULL) { // Ciclo hasta terminar de leer el archivo
        sscanf(line, "%[^,], %d, %d", nombre, &isbn, &copias); //Lee una línea y guarda los datos
        Libro libro;
        strcpy(libro.nombre, nombre);
        libro.isbn = isbn;
        for(int i=0; i < copias; i++){
            
            fgets(line, MAX_LINE, archivo_bd); // Lee los datos de cada copia
            char fecha[100];
            sscanf(line, "%d, %c, %[^,]", &libro.ejemplar, &libro.estado, fecha); // Guarda los datos de cada copia

            strptime(fecha, "%d-%m-%Y", &libro.fecha);
            bd[totalLibros] = libro; //Agrega el libro a la base de datos
            totalLibros++; // Aumenta el contador de libros
        }
    }

    if (archivoBD != NULL) {
        fclose(archivo_bd);
    }

    return;
} 


/*
 * Función encargada de buscar un libro en la
 * base de datos y retornar true si lo encuentra, false si no.
 */
bool buscar_libro(int isbn, char* nombre){

    for(int i = 0; i < totalLibros; i++) // Recorre la base de datos para buscar un libro 
        if(bd[i].isbn == isbn && strcmp(bd[i].nombre, nombre) == 0){
            return true; // Retorna true si lo encuentra
        }
    return false; // Retorna false si no lo encuentra
}

/*
 * Función que ejecuta el hilo auxiliar1 para procesar
 * solicitudes de devolver y renovar bd.
 */
void *auxiliar1_(void *arg) {
    
    while(1) {

        pthread_mutex_lock(&mutexBuffer);

        while (contador == 0)
            pthread_cond_wait(&noVacio, &mutexBuffer);
        
        if(contador == -1) //Verifica si el hilo auxiliar2 le indicó que se cierre
            break;

        // Procesar solicitud
        Solicitud soli = buffer[out]; // Lee una solicitud del búfer
        out = (out + 1) % BUFFER_SIZE; // Cambia la posición para sacar la próxima solicitud
        contador--; 

        pthread_cond_signal(&noLleno); // Envía señal para que el hilo principal sepa que el búfer no está lleno
        pthread_mutex_unlock(&mutexBuffer);

        int fifoRespuesta = open(soli.pipe, O_WRONLY); //Abre el descriptor del pipe de respuestas
        char mensaje[MAX_LINE];
        
        
        //Buscar el libro de la operación
        pthread_mutex_lock(&mutexBD); // Mutex que asegura que solo un hilo acceda a la base de datos

        if(!buscar_libro(soli.isbn, soli.libro)){
            snprintf(mensaje, sizeof(mensaje), "Libro no encontrado: %s, %d", soli.libro, soli.isbn);
            write(fifoRespuesta, mensaje, strlen(mensaje) + 1);
            pthread_mutex_unlock(&mutexBD); // Desbloquea el mutex de la Base de Datos
            continue;
        }
        
        // Aquí se actualizaría la BD según la operación
        soli.operacion == 'D' ? devolver(soli, fifoRespuesta) : renovar(soli, fifoRespuesta);
        pthread_mutex_unlock(&mutexBD); // Desbloquea el mutex de la Base de Datos
    }

    return NULL;

}

/*
* Función que procesa la devolución con un libro previamente prestado
*/
void devolver(Solicitud soli, int fifoRespuesta){

    char mensaje[MAX_LINE];

    //Busca si el libro fue prestado
    Libro* devolucion = buscarLibroPrestado(soli);

    if(devolucion == NULL){
        //Si no se encuentra, notifica que no se puede completar la operación
        snprintf(mensaje, sizeof(mensaje), "No hay %s, %d prestados, no se pudo completar la operacion", soli.libro, soli.isbn);
        write(fifoRespuesta, mensaje, sizeof(mensaje));
        return;
    }
    
    //Cambia el estado del libro a 'D' (Devuelto)
    devolucion->estado = 'D';

    //Captura la fecha actual del sistema
    time_t ahora = time(NULL);
    struct tm *local = localtime(&ahora);
    devolucion->fecha = *local;

    //Asegura exclusividad para modificar el reporte con un candado
    pthread_mutex_lock(&lock);
    
    // Guarda una copia del libro devuelto en el arreglo de reportes
    for(int i = 0; i < 100; i++)
        if(reporte[i].isbn == 0){
            reporte[i] = *devolucion;
            break;
        }
    //Libera el candado       
    pthread_mutex_unlock(&lock);

    //Envia mensaje de confirmación al solicitante
    snprintf(mensaje, sizeof(mensaje), "Libro %s, %d devuelto", devolucion->nombre, devolucion->isbn);
    write(fifoRespuesta, mensaje, sizeof(mensaje));

}

/*
* Función que procesa la renovación del prestamo de una libro previamente prestado.
*/
void renovar(Solicitud soli, int fifoRespuesta){

    char mensaje[MAX_LINE];
    Libro* renovacion = buscarLibroPrestado(soli); //Busca si el libro fue prestado

    if(renovacion == NULL){
        //Si no se encuentra, se informa que la operación no se puede completar
        snprintf(mensaje, sizeof(mensaje), "No hay %s, %d prestados, no se pudo completar la operacion", soli.libro, soli.isbn);
        write(fifoRespuesta, mensaje, sizeof(mensaje));
        return;
    }

    //Captura la fecha actual y la asigna como nueva fecha de prestamo
    time_t ahora = time(NULL);
    struct tm *local = localtime(&ahora);
    renovacion->fecha = *local;
    
    //Añade 7 dias a la fecha actual
    renovacion->fecha.tm_mday += 7;
    mktime(&renovacion->fecha); 


    //Pone un candado para guardar la operacion en el reporte
    pthread_mutex_lock(&lock);
    // Guardar operación en el reporte
    for(int i = 0; i<100; i++){
        if(reporte[i].isbn == 0){ //Encuentra un espacio libre en el reporte
            reporte[i] = *renovacion;
            break;
        }
    }

    //Libera el candado
    pthread_mutex_unlock(&lock);

    //Convierte la nueva fecha a cadena de texto con el formato adecuado
    char fecha[20];
    strftime(fecha, sizeof(fecha), "%d-%m-%Y", &renovacion->fecha);

    //Envia mensaje de confirmación al cliente
    snprintf(mensaje, sizeof(mensaje), "Prestamo del libro %s, %d aumentado a la fecha %s", renovacion->nombre, renovacion->isbn, fecha);
    write(fifoRespuesta, mensaje, sizeof(mensaje));

}

/*
* Función que busca un libro prestado en la base de datos   
*/
Libro* buscarLibroPrestado(Solicitud soli){
    
    Libro* prestado = NULL;

    //Recorre la base de datos de libros
    for(int i = 0; i < totalLibros; i++)
        if(bd[i].isbn == soli.isbn) //Coincide el ISBN
            if(bd[i].estado == 'P') //El libro esta prestado 'p'
                prestado = &bd[i];  //Guarda apuntador al libto

    return prestado; //Retorna apuntador al libro prestado o NULL si no se encontro

}


/*
 * Función encargada de procesar la solicitud de prestamo
 * de un libro y retorna la fecha de devolución si se 
 * logra o NULL si no.
 */
void prestar_libro(Solicitud solicitud, char fecha_devolucion[20]){
    
    for(int i = 0; i < totalLibros; i++){

        if(bd[i].isbn == solicitud.isbn && (strcasecmp(bd[i].nombre, solicitud.libro) == 0) && bd[i].estado == 'D'){

            bd[i].estado = 'P';

            time_t ahora = time(NULL);
            struct tm *local = localtime(&ahora);

            local->tm_mday += 7; // Sumar 7 días
            mktime(local); // Normaliza la fecha
            bd[i].fecha = *local;
            strftime(fecha_devolucion, 20, "%d-%m-%Y", local);
            
            pthread_mutex_lock(&lock);
            // Guardar operación en el reporte
            for(int j = 0; j<100; j++)
                if(reporte[j].isbn == 0){
                    reporte[j] = bd[i];
                    break;
                }
            pthread_mutex_unlock(&lock);
            
            return;

        }

    }

    fecha_devolucion[0] = '\0'; // No se pudo prestar el libro

}

void* auxiliar2_(void* arg) {

    char comando[2];

    printf("\n====== MENÚ RECEPTOR =====\n");
    printf("Ingrese s para salir\n");
    printf("Ingrese r para ver reporte\n");
    printf("==========================\n");

    while (contador != -1) {

        if (fgets(comando, sizeof(comando), stdin) != NULL) {
            if (comando[0] == 's') {
                printf("Finalizando programa...\n");

                // Envía señal para cerrar el hilo auxiliar1
                contador = -1;
                pthread_cond_signal(&noVacio); // Desbloquea el hilo auxiliar1

                // Envía solicitud de salida para desbloquear el read() en main
                Solicitud fin = {0};
                fin.operacion = 'S'; // 'S' para salir
                write(idPipeSolicitudes, &fin, sizeof(Solicitud));
                break;
                
            } else if (comando[0] == 'r') {
                // Mostrar reporte de solicitudes
                pthread_mutex_lock(&lock);
                printf("\n==== Reporte de Operaciones Realizadas ====\n");
                for (int i = 0; i < 100; i++) {
                    if (reporte[i].isbn != 0) {
                        char fecha[20];
                        strftime(fecha, sizeof(fecha), "%d-%m-%Y", &reporte[i].fecha);
                        printf("Estado: %c, Libro: %s, ISBN: %d, Ejemplar: %d, Fecha: %s\n", reporte[i].estado ,reporte[i].nombre, reporte[i].isbn, reporte[i].ejemplar, fecha);
                    }
                    
                }
            }
        }
    }

    return NULL;

}

void prestamo(Solicitud solicitud){

    char mensaje[MAX_LINE];

    // Procesar préstamo directamente
    int fifo_respuesta = open(solicitud.pipe, O_RDWR);

    char fecha_devolucion[20] = "";
    prestar_libro(solicitud, fecha_devolucion);
            
    if(fecha_devolucion[0] != '\0') {
        snprintf(mensaje, sizeof(mensaje), "Se prestó el libro %s, hasta la fecha %s", solicitud.libro, fecha_devolucion);
        write(fifo_respuesta, mensaje, sizeof(mensaje));
    } else {
        snprintf(mensaje, sizeof(mensaje), "Libro %s no disponible", solicitud.libro);
        write(fifo_respuesta, mensaje, sizeof(mensaje));
    }

}

void devolverRenovar(Solicitud solicitud){

    // Agregar al búfer para el hilo auxiliar
    pthread_mutex_lock(&mutexBuffer);

    while (contador == BUFFER_SIZE)
        pthread_cond_wait(&noLleno, &mutexBuffer);
    

    buffer[in] = solicitud;
    in = (in + 1) % BUFFER_SIZE;
    contador++;

    pthread_cond_signal(&noVacio);
    pthread_mutex_unlock(&mutexBuffer);

}

/*
* Función que actualiza el contador global de solicitantes activos
*/
void actualizar_solicitante(Solicitud sol){

    if(sol.operacion == 'Q'){
        //Solicitante se desconecta
        solicitantesActivos--;
    } else {
        //Nuevo solicitante
        solicitantesActivos++;
    }   
}

/*
* Función que actualiza la base de datos
*/
void actualizar_bd(char* archivoBD){    
    int isbn_vistos[100] = {0};
    char nombres_vistos[100][100];
    int contadores[100] = {0};
    int distintos = 0;

    FILE *archivo = fopen(archivoBD, "w");
    if(archivo == NULL){
        perror("fopen");
        return;
    }

    for(int i=0; i < totalLibros; i++){
        int encontrado = 0;
        for(int j=0; j < distintos; j++){
            if(bd[i].isbn == isbn_vistos[j] && strcmp(bd[i].nombre, nombres_vistos[j]) == 0){
                contadores[j]++;
                encontrado = 1;
            }
        }

        if(!encontrado){
            isbn_vistos[distintos] = bd[i]. isbn;
            strcpy(nombres_vistos[distintos], bd[i].nombre);
            contadores[distintos] = 1;
            distintos++;
        }
    }

    for(int i=0; i < distintos; i++){
        fprintf(archivo, "%s, %d, %d\n", nombres_vistos[i], isbn_vistos[i], contadores[i]);
        
        for(int j=0; j < totalLibros; j++){
            if(bd[j].isbn == isbn_vistos[i] && strcmp(bd[j].nombre, nombres_vistos[i]) == 0){
                char fecha[20];
                strftime(fecha, sizeof(fecha), "%d-%m-%Y", &bd[j].fecha); 
                fprintf(archivo, "%d, %c, %s\n" , bd[j].ejemplar, bd[j].estado, fecha);
            }
        }
    }

    fclose(archivo);
}

/*
* Función que genera un reporte y lo escribe en un archivo de texto
*/
void escribir_reporte(const char* nombre_archivo) {

    FILE* archivo = fopen(nombre_archivo, "w");
    if (!archivo) {
        perror("No se pudo abrir el archivo");
        return;
    }
    
    int procesados[100] = {0};  //Marcar cuáles bd ya fueron procesados

    for (int i = 0; i < totalLibros; i++) {
        if (procesados[i]) continue;

        char* titulo = bd[i].nombre;
        int isbn = bd[i].isbn;

        // Contar total y disponibles
        int total = 0, disponibles = 0;

        //Recorre todos los libros para contar ejemplares con mismo titulo e ISBN
        for (int j = 0; j < totalLibros; j++) {
            if (!procesados[j] && strcmp(bd[j].nombre, titulo) == 0 && bd[j].isbn == isbn) {
                total++;
                if (bd[j].estado == 'D') disponibles++;
                procesados[j] = 1;
            }
        }

        // Escribir cabecera del libro
        fprintf(archivo, "%s, %d, %d\n", titulo, isbn, total);
        fprintf(archivo, "Ejemplares Disponibles: %d\n", disponibles);

        //Detalla cada ejemplar individualmente
        for (int j = 0; j < totalLibros; j++) {
            if (strcmp(bd[j].nombre, titulo) == 0 && bd[j].isbn == isbn) {
                fprintf(archivo, "%d, %c, %d-%d-%d\n",
                        bd[j].ejemplar,
                        bd[j].estado,
                        bd[j].fecha.tm_mday,
                        bd[j].fecha.tm_mon + 1,
                        bd[j].fecha.tm_year + 1900);
            }
        }

        fprintf(archivo, "\n");  // Separar entre libros
    }

    fclose(archivo); //Cerrar archivo
}