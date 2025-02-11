/*
****************************************************************************************************
* CESE 2020  Co12  TP2 
****************************************************************************************************
* 
* archivo: server.h
*
* version: 1.0
*
* autor: Felipe A. Calcavecchia
*
* fecha: 05/12/2020
*
****************************************************************************************************
*/

/* --------------------------------------- definicion de ctes -------------------------------------- */

#define BACKLOG                 10

#define IP                      "127.0.0.1"
#define PORT                    (10000)

#define MSJ_L                   100
#define BUF_L                   16

#define DELIMITADOR             ":"
#define PREFIJO_TX              ">OUT"


void* threadServidor (void* p);