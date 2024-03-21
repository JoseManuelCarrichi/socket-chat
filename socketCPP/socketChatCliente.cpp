#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <string.h>
 
#define SERVER_PORT	6543
#define SERVER_ADDRESS	"127.0.0.1"
#define MAXLINE		512
 
void* recibir(void* p);
void* enviar(void* p);
 
int main()
{
	struct sockaddr_in addr;
	int sd,status;
	pthread_t hilos[2];
 
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
	addr.sin_port = htons(SERVER_PORT);
 
	if((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("Error al crear el socket\n");
		exit(0);	
	}	
	if(connect(sd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
	{
		printf("Error al conectar\n");
		exit(0);
	}
	else
	{	
		if ( (status = pthread_create(&hilos[0],NULL,recibir,(void*)&sd)) )
		{
			printf("Error al crear hilo para recibir\n");
			close(sd);
			exit(0);
		}		
		if ( (status = pthread_create(&hilos[1],NULL,enviar,(void*)&sd)) )
		{
			printf("Error al crear hilo para enviar\n");
			close(sd);
			exit(0);
		}
		pthread_join(hilos[0],NULL);
		pthread_join(hilos[1],NULL);		
	}
	return 1;
}
 
void* recibir(void* p)
{
	int* id;
	char buffer[MAXLINE];
	id = (int*) p;
	while(1)
	{
		recv(*id,buffer,MAXLINE,0);
		printf("%s\n",buffer);
		fflush(stdout);
	}
}
 
void* enviar(void* p)
{
	int* id;
	char buffer[MAXLINE];
	id = (int*) p;
	while(1)
	{	 
		printf("\tIntroduce el mensaje\n-> ");
		fgets(buffer , MAXLINE , stdin);
		strtok(buffer,"\n");
		send(*id,buffer,MAXLINE,0);
		sleep(3);
	}
}