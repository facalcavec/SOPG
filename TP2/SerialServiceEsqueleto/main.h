/*
****************************************************************************************************
* CESE 2020  Co12  TP2 
****************************************************************************************************
* 
* archivo: main.h
*
* version: 1.0
*
* autor: Felipe A. Calcavecchia
*
* fecha: 05/12/2020
*
****************************************************************************************************
*/

#include "buffer.h"

#define BAUDRATE                115200
#define TTY                     1
#define TTY_TEXTO               "/dev/ttyUSB1"
#define SERIAL_BUF_L            16
#define DELIMITADOR             ":"
#define PREFIJO_RX              ">SW"

#define MENSAJE_L               80
#define CONEXION_ESTABLECIDA    1
#define CONEXION_PERDIDA        0

#define ERROR_SIGINT            -2
#define ERROR_SIGTERM           -3
#define ERROR_CANCEL_THREAD     -4
#define ERROR_JOIN              -5
#define ERROR_WRITE_SOCKET      -6
#define ERROR_READ_SOCKET       -7
#define ERROR_BLOCK_SIGMASK     -8
#define ERROR_UNBLOCK_SIGMASK   -9
#define ERROR_FD                -10
#define ERROR_SOCKET            -11

/*-------- prototipos de funciones -------------*/
int abrirPuertoSerie (void);
int lanzarThreadServidor (pthread_t *pServidor);
int inicializarBuf (buf_t *buf);
int escribirBuf (buf_t * buf, char* cadena );
int leerBuf (buf_t * buf, char* cadena );
void bloquearSign( void );
void desbloquearSign( void );
void sig_handler( int sig );