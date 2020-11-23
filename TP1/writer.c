/*
 * file: writer.c
 * 
 * SOPG // TP1
 *
 * 15/11/2020
 */

/* ------------------------ inclusion de archivos ------------------------------ */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <signal.h>

#include "writer.h"
//#include "cadenastipo.h"

/* ------------------------ variables globales ------------------------------ */
volatile sig_atomic_t fd;

char cadenaSIGUSR1[10];
char cadenaSIGUSR2[10];


int main(void)
{
	char cadena[CADENA_SIZE];
	char*cadenaAux;
	char cadenaDatos[CADENA_SIZE];
	int bytesWrote;
	pid_t miPID;
    
	struct sigaction sa;
	struct sigaction si;
	struct sigaction sa_sigusr1;
	struct sigaction sa_sigusr2;

	printf("Sistemas Operativos de Proposito General\n");
	printf("Trabajo Practico 1 -> WRITER\n\n");

	miPID = getpid();           // obtengo el pid de este proceso
	printf("El PID del proceso es: %d\n\n", miPID);

	printf("Instalando handlers de señales:\n");

	/* ------ instalo sigint ------ */
	//printf("Instalando handler de SIGINT...\n");
	si.sa_handler = sigint_handler;
	si.sa_flags = 0;
	sigemptyset(&si.sa_mask);
	if (sigaction(SIGINT, &si, NULL) == -1) 
	{
		perror("Error al instalar handler de SIGINT");
		exit(1);
	}
	printf("Handler de SIGINT instalado\n");


	/* ------ instalo sigpipe ------ */
	sa.sa_handler = sigpipe_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGPIPE, &sa, NULL) == -1) 
	{
		perror("Error al instalar handler de SIGPIPE");
		exit(1);
	}
	printf("Handler de SIGPIPE instalado\n");

	/* ------ instalo sigusr1 ------ */
	sa_sigusr1.sa_handler = sigusr1_handler;
	sa_sigusr1.sa_flags = SA_RESTART;
	sigemptyset(&sa_sigusr1.sa_mask);
	if (sigaction(SIGUSR1, &sa_sigusr1, NULL) == -1) 
	{
		perror("Error al instalar handler de SIGUSR1");
		exit(1);
    	}
		printf("Handler de SIGUSR1 instalado\n");

	/* ------ instalo sigusr2 ------ */
	sa_sigusr2.sa_handler = sigusr2_handler;
	sa_sigusr2.sa_flags = SA_RESTART;
	sigemptyset(&sa_sigusr2.sa_mask);

	if (sigaction(SIGUSR2, &sa_sigusr2, NULL) == -1) {
		perror("Error al instalar handler de SIGUSR2");
		exit(1);
	}
	printf("Handler de SIGUSR2 instalado\n");

	/* ------ creo fifo ------ */
	printf("\nAccediendo a FIFO : %s ...\n", FIFO_NAME);
	if(mknod(FIFO_NAME, S_IFIFO | 0666, 0) != 0) 
	{
    
        	if(errno == EEXIST)
			printf("Ya existe un archivo myfifo\n\n");
        	else 
		{
			perror("Error al crear la FIFO");
			return ERROR_MKNOD;
        	}
	}
	else 
        	printf("Archivo myfifo creado por WRITER\n\n");

	/* ------ abro fifo ------ */
	printf("Esperando por lectores...\n");
	fd = open(FIFO_NAME, O_WRONLY);

	if(fd == -1)
	{
		perror("Error al abrir la FIFO.\n");
		return ERROR_OPEN_FIFO;
	}

    

	/* ------ genero cadenas sigusr ------ */
	sprintf(cadenaSIGUSR1, "%s%s", PREFIJO_SIGUSRx, MENSAJE_SIGUSR1);
	sprintf(cadenaSIGUSR2, "%s%s", PREFIJO_SIGUSRx, MENSAJE_SIGUSR2);

	/* ------ loop ppal: ------ */


	printf("Programa READER conectado\n");
    	printf("------------------------------ \n\n\n");

	while (1)
	{
        	printf(MSJ_INGRESE_TEXTO);
		cadenaAux = fgets(cadena, CADENA_SIZE, stdin);

        	cadenaAux [strcspn(cadenaAux, "\n")] = '\0';    // saco el \n que mete el fgets

        	if(cadenaAux != cadena) 
		{
            		perror("Error al leer del buffer stdin.\n");
            		close(fd);
            		return ERROR_FGETS;
        	}

        	sprintf(cadenaDatos, "%s%s", PREFIJO_TEXTO, cadenaAux);

		if ((bytesWrote = write(fd, cadenaDatos, strlen(cadenaDatos))) == -1) 
		{
			perror("Error al escribir en la FIFO.");
            		return ERROR_ESCRIBIR_FIFO;
            
        	}
		else
			printf("Se escribieron %d bytes\n", bytesWrote);
	}
	return 0;
}


/* ------------------------ Funciones ------------------------------ */


/* Funcion que captura SIGINT cuando quieran cerrar el programa con Ctrl+C */

void sigint_handler(int sig)
{
	write(0, MSJ_SALIDA_SIGINT, sizeof(MSJ_SALIDA_SIGINT));
	close(fd);
	exit(ERROR_SIGINT);
}



/* Funcion que captura el SIGPIPE para el caso en que se cierre el reader y se quiera escribir en la fifo */

void sigpipe_handler(int sig)
{
	write(0, MSJ_SALIDA_SIGPIPE, sizeof(MSJ_SALIDA_SIGPIPE));
	close(fd);
	exit(ERROR_READER_CERRADO);
}


/* Funcion que toma SIGUSR1 para enviar a reader */

void sigusr1_handler(int sig)
{
	int bytesWrote;

	if ((bytesWrote = write(fd, cadenaSIGUSR1, strlen(cadenaSIGUSR1)+1)) == -1) {
		perror("Error al escribir en la FIFO.");
		exit(ERROR_ESCRIBIR_SIGUSR1);
	}
	else 
	{
		write(0, MSJ_TERMINAL_SIGUSR1, sizeof(MSJ_TERMINAL_SIGUSR1));    
		write(0, MSJ_INGRESE_TEXTO, sizeof(MSJ_INGRESE_TEXTO));
	}
	return;
}


/* Funcion que toma SIGUSR2 para enviar a reader */

void sigusr2_handler(int sig)
{
	int bytesWrote;
    
    	if ((bytesWrote = write(fd, cadenaSIGUSR2, strlen(cadenaSIGUSR2)+1)) == -1) 
	{
		perror("Error al escribir en la FIFO.");
        	exit(ERROR_ESCRIBIR_SIGUSR2);
    	}
	else 
	{
        write(0, MSJ_TERMINAL_SIGUSR2, sizeof(MSJ_TERMINAL_SIGUSR2));  
        write(0, MSJ_INGRESE_TEXTO, sizeof(MSJ_INGRESE_TEXTO));  
    	}
    	return;
}

