/*
****************************************************************************************************
* CESE 2020  Co12  TP2 
****************************************************************************************************
* 
* archivo: server.c
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
#include <string.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "main.h"
#include "server.h"
#include "SerialManager.h"


int n;
char servidor_rx_buf[BUF_L];
int bytes_enviar;
extern buf_t bufSerieTx;
extern bool connection;
extern bool loop_in;

/* funcion que lanza el thread del servidor */
int lanzarThreadServidor (pthread_t *pServidor) 
{
    printf( "Generando thread para iniciar Servidor\n" );

    if( pthread_create (pServidor, NULL, threadServidor, NULL )) 
    {
        perror( "lanzarThreadServidor - pthread_create()");
        return 1;
    }

    return 0;
}

/* thread que inicia socket para conexion */
void* threadServidor ( void* p ) 
{
    socklen_t addr_len;
    struct sockaddr_in clientaddr;
    struct sockaddr_in serveraddr;

    char mensaje[MSJ_L];
    char buffer[BUF_L];
    
    buf_t bufServerRx;

    extern int newfd;

    int n;


    /* Creando socket */
    int s = socket( PF_INET,SOCK_STREAM, 0 );

    /* Carga datos de IP:PORT del server */
    printf( "\nGenerando socket para recibir conexion..." );
    bzero(( char * ) &serveraddr, sizeof( serveraddr ));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = inet_addr( IP );

    if( serveraddr.sin_addr.s_addr == INADDR_NONE )
    {
        sprintf( mensaje, "Error al generar el IP %s", IP );
        perror( mensaje );
        return 0;
    }
    printf( "se genero la direccion IP correctamente\n" );


    /* Abre puerto con bind() */
    printf("Abriendo puerto..."); 

    if (bind( s, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) == -1 ) 
    {
        close( s );
        sprintf( mensaje, "Error al abrir el puerto %d", PORT );
        perror( mensaje );
        return 0;
    }
    printf( "el puerto se abrio correctamente\n" );

    /* Setea el socket en modo Listening */
    printf( "Configurando socket en modo <<listen>>..." );

    if ( listen (s, BACKLOG) == -1 ) // backlog=BACKLOG
    {
        close( s );
        perror( "Error al configurar socket en modo listen" );
        exit( 1 );
    }
    printf( "socket configurado en modo <<listen>>\n" );

    printf( "----------------------------------------------\n\n" );
    printf( "Esperando nuevas conexiones a %s:%d...\n\n", IP, PORT );
    
    /* primer bucle que solo se ejecuta cuando se perde la conexion  */
    while( loop_in )
    {
        /* Ejecuta el accept() para recibir conexiones entrantes */
        addr_len = sizeof( struct sockaddr_in );

        newfd = accept( s, ( struct sockaddr * )&clientaddr, &addr_len );
        
        if ( newfd == -1 ) 
        {
            perror( "Error en accept" );
            exit( ERROR_READ_SOCKET );    
        }
        
        connection = CONEXION_ESTABLECIDA;
        
        printf( "\nNueva conexion entrante establecida desde:  %s:%d\n\n", inet_ntoa( clientaddr.sin_addr ), clientaddr.sin_port );   
        printf( "----------------------------------------------\n\n" );
        printf( "Esperando datos de la CIAA o el cliente...\n\n" );

        /* segundo bucle que se ejecuta y esperar la llegada de un dato */
        while( loop_in )
        {
            {
                n = read( newfd, servidor_rx_buf, BUF_L );
                
                if( n == -1 )
                {
                    perror( "Thread: error leyendo mensaje en socket" );
                    exit( 1 );
                }

                servidor_rx_buf[n] = '\0';
                
                printf( "El cliente envio %d bytes: %s\n", n, servidor_rx_buf );

                if( n > 0)
                {                      
                    char token[BUF_L];

                    strcpy( token, servidor_rx_buf ); 
                    strtok( token, DELIMITADOR );           // separa el string en dos partes por el delimitador ":"       
                                
                    if( !strcmp( token, PREFIJO_TX ))       // acepta si el cmd coincide con el prefijo
                    {                        
                        if( escribirBuf( &bufSerieTx, servidor_rx_buf )) 
                        {
                            printf( "No se pudo escribir el buffer de recepcion del puerto Serie\n" );
                        }
                    }
                    else 
                    {
                        printf( "Dato sin formato correcto" );
                    }
                }
                
                /* si llega un EOF se cierra la conexion */
                if( n == 0 ) 
                {
                    printf( "Cerrando la conexion del cliente..." );
                    break;
                }    
            }                         
        }
        
        /* Cerramos conexion con cliente */
        if( close( newfd ) == -1)
        {
            perror("Error al cerrar file descriptor/n");
            exit(ERROR_FD);
        }
        connection = CONEXION_PERDIDA;
        printf( "Se cerro la conexion con cliente\n" );
    }
    
    printf("Cierro hilo.\n\n" );
    return 0;
}