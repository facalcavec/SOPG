/*
****************************************************************************************************
* CESE 2020  Co12  TP2 
****************************************************************************************************
* 
* archivo: buffer.h
*
* version: 1.0
*
* autor: Felipe A. Calcavecchia
*
* fecha: 05/12/2020
*
****************************************************************************************************
*/

#include <pthread.h>

/* --------------------------------------- definicion de ctes. -------------------------------------- */
#define BUF_DATOS               16


/* --------------------------------------- tipos de datos -------------------------------------- */
typedef enum {DATOS_NO_LEIDOS, DATOS_LEIDOS, DATOS_INACTIVO} estadoBufDatos_t;

typedef struct {

    pthread_mutex_t mutexBuf;
    estadoBufDatos_t estadoBufDatos;
    char bufDatos [BUF_DATOS];
} buf_t;

/* --------------------------------------- variables globales -------------------------------------- */

extern int inicializarBuf (buf_t *buf);
extern int escribirBuf (buf_t * buf, char* cadena );
extern int leerBuf (buf_t * buf, char* cadena );
