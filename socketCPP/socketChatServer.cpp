#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
 
#define SERVER_PORT	6543
#define SERVER_ADDRESS	"127.0.0.1"
#define MAXLINE		512
#define MAXCLIENTS	10
 
int buscarCliente(char*);
void subCadena(char*, char*, int, int);
 
struct vector
{
	int socket;
	char usuario[MAXLINE-4];
	int sign_in;
};
 
struct vector vectorClientes[MAXCLIENTS];
int clientes = 0;
 
int main(int argc, char *argv[])
{
    void* gestionaCliente(void* p);
 
    int socketfd, new_sd;
    socklen_t client_len;
    struct sockaddr_in server_addr, client_addr;
    int i, status, id;
    pthread_t hilos[MAXCLIENTS];
 
    for(i=0;i<MAXCLIENTS;i++)
	strcpy(vectorClientes[i].usuario, " ");
 
    // Open TCP internet STREAM socket
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	perror("server: Can't open stream socket");
 
    // Bind local address to allow the client to connect
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
    if (bind
	(socketfd, (struct sockaddr *) &server_addr,
	 sizeof(server_addr)) < 0)
	perror("server: can't bind local address");
    listen(socketfd, 5);
 
    for (;;) {
	client_len = sizeof(client_addr);
	if((new_sd = accept(socketfd, (struct sockaddr *) &client_addr, &client_len)) < 0) {
		printf("Error aceptando peticiones\n");
		exit(0);
	}	
	else {	
		id = clientes;		
		vectorClientes[id].socket = new_sd;
		vectorClientes[id].sign_in = 0;		
		fflush(stdout);			
		clientes++;
		if ( (status = pthread_create(&hilos[id],NULL,gestionaCliente,(void *)&id)) )
		{
			printf("Error al crear el hilo\n");
			exit(0);
		}
	}
   }
   close(socketfd);	// Close original socket
   return 0;
}
 
void subCadena(char *subCad, char *cad, int inicio, int cuantos)
{
     int i,j=0;
     for(i=inicio;i<inicio+cuantos && cad[i]!='\0';i++)
     {
        subCad[j]=cad[i];
        j++;
     }
     subCad[j]='\0';
}
 
int buscarCliente(char* usuario)
{
	int i;
	for(i=0;i<clientes;i++)
	{
		if(strcmp(vectorClientes[i].usuario,usuario ) == 0 && vectorClientes[i].sign_in == 1)
		return vectorClientes[i].socket;
	}
	return -1;
}
 
void* gestionaCliente(void* p)
{
	int *ide, id;
	ide = (int* ) p;
	id = *ide;
	char buffer[MAXLINE], nombre[MAXLINE-4], temp[MAXLINE-8];
	int i, longitud, destino;
 
	while(1)
	{
	recv(vectorClientes[id].socket,buffer,MAXLINE,0);
	printf("\nid%d\n", id);
 
	if(strstr(buffer, "ADD") && vectorClientes[id].sign_in == 0) {
		longitud = strlen(buffer);
		//Le quitamos el ADD
		subCadena(nombre, buffer, 4, longitud-4);
		strcpy(vectorClientes[id].usuario, nombre);
		//Se informa a todos menos a él mismo y al que se haya ido
		strcpy(buffer, "El usuario ");
		strcat(buffer, nombre);
		strcat(buffer, " ha entrado en el chat.");
		for(i = 0; i < clientes; i++)
			if (i != id && vectorClientes[i].sign_in == 1)
			send(vectorClientes[i].socket,buffer,MAXLINE,0);
		vectorClientes[id].sign_in = 1;
		}
	if(strstr(buffer, "LIST") && vectorClientes[id].sign_in == 1) {
		//Se envia al cliente todos los usuarios menos los que hayan abandonado la sesión
		for(i = 0; i < clientes; i++){
			if(vectorClientes[i].sign_in == 1)
				send(vectorClientes[id].socket, vectorClientes[i].usuario, MAXLINE, 0);			
			}		
		}
	if(strstr(buffer, "END") && vectorClientes[id].sign_in == 1) {
		//Se informa a todos menos a él mismo y al que se haya ido
		strcpy(buffer, "El usuario ");
		strcat(buffer, vectorClientes[id].usuario);
		strcat(buffer, " ha abandonado en el chat.");
		bzero(vectorClientes[id].usuario, MAXLINE);
		for(i = 0; i < clientes; i++)
			if (i != id && vectorClientes[i].sign_in == 1)
				send(vectorClientes[i].socket,buffer,MAXLINE,0);
		vectorClientes[id].sign_in = 0;
		}
	if(strstr(buffer, "TEXT") && !strstr(buffer, "TEXT TO") && vectorClientes[id].sign_in == 1) {
		longitud = strlen(buffer);
		subCadena(temp, buffer, 5, longitud-5);
		//Se envía a todos menos a él mismo y al que se haya ido
		bzero(buffer, MAXLINE);
		strcat(buffer, vectorClientes[id].usuario);
		strcat(buffer, " dice: ");
		strcat(buffer, temp);
		for(i = 0; i < clientes; i++)
			if (i != id && vectorClientes[i].sign_in == 1)
			send(vectorClientes[i].socket,buffer,MAXLINE,0);
		}
	if(strstr(buffer, "TEXT TO") && vectorClientes[id].sign_in == 1) {
		//Le quitamos el TEXT TO
		subCadena(nombre, buffer, 8, MAXLINE-8);
		//Nos quedamos sólo con el nombre, quitando desde el primer espacio en blanco hasta el final
		strtok(nombre," ");
		//Se obtiene el socket destino
		destino = buscarCliente(nombre);
		longitud=strlen(nombre);
		strcpy(nombre, vectorClientes[id].usuario);		
		strcat(nombre, " dice: ");
		//Recortamos el TEXT TO, el nombre, y los dos espacios hasta el mensaje(se suma solo uno (un espacio) 
		//porque empieza a recorrer longitud(instruccion)+longitud(nombre) desde la pos. 0 de la cadena.		
		subCadena(temp, buffer, 8+longitud+1, MAXLINE-(8+longitud+1));
		strcat(nombre, temp);
		if(destino != -1)
			send(destino, nombre, MAXLINE, 0);
		}
	fflush(stdout);
	}
	close(vectorClientes[id].socket);
}