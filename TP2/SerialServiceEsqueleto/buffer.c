/*
****************************************************************************************************
* CESE 2020  Co12  TP2 
****************************************************************************************************
* 
* archivo: buffer.c
*
* version: 1.0
*
* autor: Felipe A. Calcavecchia
*
* fecha: 05/12/2020
*
****************************************************************************************************
*/

/* --------------------------------------- inclusion de archivos -------------------------------------- */
#include <string.h>
#include <stdio.h>

#include "buffer.h"

/* --------------------------------------- prototipos -------------------------------------- */

int inicializarBuf (buf_t *buf);
int escribirBuf (buf_t * buf, char* cadena );
int leerBuf (buf_t * buf, char* cadena );

/* --------------------------------------- funciones -------------------------------------- */


/* inicializa estructura de datos */
int inicializarBuf (buf_t *buf) {

    pthread_mutex_init(&buf->mutexBuf, NULL);
    memset((void*) buf->bufDatos, 0, BUF_DATOS);    
    buf->estadoBufDatos = DATOS_INACTIVO;

    return 0;
}


/* escribe una cadena al buffer y levanto el estado */
int escribirBuf (buf_t * buf, char* cadena ) 
{
       
    if(pthread_mutex_lock (&buf->mutexBuf)) 
    {
        printf( "escribirBuf: no pudo bloquear el mutex\n" );
        return 1;
    }

    strcpy(buf->bufDatos, cadena);
    buf->estadoBufDatos = DATOS_NO_LEIDOS;

    if(pthread_mutex_unlock (&buf->mutexBuf)) 
    {
        printf( "escribirBuf: no pudo desbloquear el mutex\n" );
        return 1;
    }

    return 0;
}


/* copia una cadena en el buffer y levanta el estado */
int leerBuf ( buf_t * buf, char* cadena ) 
{
    int n;

    if( pthread_mutex_lock (&buf->mutexBuf )) 
    {
        printf( "leerBuf: no pudo tomar el mutex\n" );
        return -1;
    }

    if( buf->estadoBufDatos != DATOS_NO_LEIDOS )
    {
        if( pthread_mutex_unlock (&buf->mutexBuf )) 
        {
            printf( "leerBuf: no pudo desbloquear el mutex\n" );
            return -1;
        }
        return -2;
    }
        
    n = strlen( buf->bufDatos );
    strcpy( cadena, buf->bufDatos );
    buf->estadoBufDatos = DATOS_LEIDOS;

    if( pthread_mutex_unlock (&buf->mutexBuf )) 
    {
        printf( "leerBuf: no pudo desbloquear el mutex\n" );
        return -1;
    }

    return n;
}