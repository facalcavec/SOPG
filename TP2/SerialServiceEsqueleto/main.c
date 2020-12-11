/*
****************************************************************************************************
* CESE 2020  Co12  TP2 
****************************************************************************************************
* 
* archivo: main.c
*
* version: 1.0
*
* autor: Felipe A. Calcavecchia
*
* fecha: 05/12/2020
*
****************************************************************************************************
*/



#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>

#include "main.h"
#include "server.h"
#include "SerialManager.h"

/* --------------------------------------- variables globales -------------------------------------- */

buf_t bufSerieTx;
int newfd;
pthread_t servidor_thread;
bool connection = CONEXION_PERDIDA;
bool loop_in = true;

int main(void)
{
	/*---------- definicion de variables locales -------------*/
    char serial_rx_buf [SERIAL_BUF_L];
    char serial_tx_buf [SERIAL_BUF_L]; 
    int bytes_recibidos;
    char servidor_tx_buf[BUF_L];
    int bytes_enviar;  
    void* ret;

    pid_t miPID;

	
    /* Obtiene el PID de este proceso */
	miPID = getpid();
	printf("Inicio Serial Service (PID: %d)\r\n", miPID);

	/* inicializacion de SIGINT */
	struct sigaction si;
	si.sa_handler = sig_handler;
	si.sa_flags = 0;
	sigemptyset( &si.sa_mask );

	if ( sigaction( SIGINT, &si, NULL ) == -1) 
    {
		perror("Error al instalar handler de SIGINT");
		exit(ERROR_SIGINT);
	}    
    
    /* inicializacion de SIGTERM */
	struct sigaction st;
	st.sa_handler = sig_handler;
	st.sa_flags = 0;
	sigemptyset( &st.sa_mask );

	if ( sigaction( SIGTERM, &st, NULL ) == -1) 
    {
		perror( "Error al instalar handler de SIGTERM" );
		exit( ERROR_SIGTERM );
	}
    
    if(inicializarBuf(&bufSerieTx)) {
        printf("Error al inicializar estructura de datos del buffer\n");
        return 1;
    }  
    
    /* configura el puerto serie */
    if( abrirPuertoSerie() ) 
    {
        printf("No se pudo abrir el puerto serie\n");
        return 1;
    }


    /* se bloquen las signals para crear el hilo servidor */
    bloquearSign();

    /* crea el hilo que maneja el servidor */
    if(lanzarThreadServidor(&servidor_thread)) 
    {
        perror ("No se pudo crear thread para iniciar el servidor\n");
        return 1;
    } 

    /* se desbloquen las signals luego de crear el hilo servidor */
    desbloquearSign();
    
    while( loop_in )
    {
        bytes_recibidos  = serial_receive(serial_rx_buf, SERIAL_BUF_L);
        
        if(bytes_recibidos < 0) 
        {
            printf("Error al leer puerto serie\n");
        }
        else if(bytes_recibidos > 0) 
        {
            serial_rx_buf[bytes_recibidos] = '\0';      // agrega NULL al final de la cadena
            char token[SERIAL_BUF_L];
            
            strcpy( token, serial_rx_buf ); 
            strtok( token, DELIMITADOR );               // separa el string en dos partes por el delimitador ":"                 
            
            if(!strcmp( token, PREFIJO_RX ))            // acepta si el cmd coincide con el prefijo
            {               
                printf( "La CIAA envio %d bytes: %s\n", bytes_recibidos, serial_rx_buf );
                
                if( connection == CONEXION_ESTABLECIDA )
                {
                    if( write( newfd, serial_rx_buf, bytes_recibidos ) == -1 )
                    {
                        perror( "Thread: error escribiendo mensaje en socket\n" );
                        exit( ERROR_WRITE_SOCKET );
                    }
                    
                    printf( "El servidor reenvio %d bytes: %s\n", bytes_recibidos, serial_rx_buf );                    
                }
                else
                {
                    printf("Se perdio la conexion con el cliente\n");
                }    
            }
            else 
            {
                printf("Dato sin formato correcto\n");
            }
        }

        if((bytes_enviar = leerBuf(&bufSerieTx, serial_tx_buf )) > 0)
        {
            /* tengo datos para enviar */
            serial_send(serial_tx_buf, bytes_enviar);

            printf( "La CIAA recibio %d bytes: %s\n", bytes_enviar, serial_tx_buf );
            printf( "------------- fin del mensaje --------------\n\n" );
            printf( "Esperando nuevo comando...\n\n" );
        }
   
        usleep(10000);
    }

    /* Cerramos conexion con cliente */
    if( connection == CONEXION_ESTABLECIDA )
    {
        if( close( newfd ) == -1)
        {
            perror("Error al cerrar file descriptor/n");
            exit(ERROR_FD);
        }
        printf( "Se cerro el socket con cliente\n\n" );
    }


    printf("Finalizando hilo servidor...." );     
    if( pthread_cancel ( servidor_thread ) != 0 )
    {
        perror("Error en la cancelacion del hilo servidor/n");
		exit(ERROR_CANCEL_THREAD);
    }   
    
    if( pthread_join( servidor_thread, &ret ) != 0)
    {
        perror("Error en join/n");
		exit(ERROR_JOIN);       
    }
    if( ret == PTHREAD_CANCELED )
    {
        printf("Se cancelo el hilo servidor.\n\n" );
    }
    else
    {
        printf("Finalizo el hilo servidor.\n\n" );
    }
    

    /* cierra puerto serie */
    serial_close();
    printf("Se cerro el puerto serie\n\n");

    printf("********** FIN DEL PROCESO **********\n\n" );

	exit(EXIT_SUCCESS);
	return 0;
}


/* --------------------------------------- funciones -------------------------------------- */

/* Funcion para abrir el puerto tty que se comunica con la CIAA */
int abrirPuertoSerie (void) {

    char mensaje [MENSAJE_L];
     
     /* abre puerto serie */
    printf("Abriendo puerto serie...");
    
    if(serial_open(TTY, BAUDRATE)) {
        sprintf(mensaje, "Error al abrir puerto. Chequear la coneccion a la CIAA? %s\n", TTY_TEXTO);
        perror(mensaje);
        return 1;        
    }
    printf("Puerto serie abierto: %s %d 8N1\n", TTY_TEXTO, BAUDRATE);

    return 0;
}

/* Funcion que enmascara las señales para bloquearlas */
void bloquearSign( void )
{
    sigset_t set;
    int s;
    sigemptyset( &set );
    sigaddset( &set, SIGINT );
    sigaddset( &set, SIGTERM );

    if( pthread_sigmask( SIG_BLOCK, &set, NULL ) != 0) 
    {
		perror( "Error al bloquear pthread_sigmask" );
		exit( ERROR_BLOCK_SIGMASK );
	}
}

/* Funcion que desenmascara las señales para desbloquearlas */
void desbloquearSign( void )
{
    sigset_t set;
    int s;
    sigemptyset( &set );
    sigaddset( &set, SIGINT );
    sigaddset( &set, SIGTERM );
    pthread_sigmask( SIG_UNBLOCK, &set, NULL );

    if( pthread_sigmask( SIG_UNBLOCK, &set, NULL ) != 0) 
    {
		perror( "Error al desbloquear pthread_sigmask" );
		exit( ERROR_UNBLOCK_SIGMASK );
	}
}

/* handler de señales */
void sig_handler( int sig )
{
	printf( "\nSe recibio un SIGINT o SIGTERM !!!\n\n" );
    
    /* Limpio loop_in para una salida ordenada */
    loop_in = false;
}
