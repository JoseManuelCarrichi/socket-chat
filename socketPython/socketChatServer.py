#!/usr/bin/python
# -*- coding: utf-8 -*-
#Servidor
 
import string
import threading
import socket
 
clientes = {'nombre':[], 'socket':[]}
 
class gestionaClientes(threading.Thread):
 
    def __init__(self, socket):
        threading.Thread.__init__(self)
        self.conn = socket
        self.conectado = False
        self.data = ''
    def run(self):
        while True:
            self.data = self.conn.recv( 1024 )
    
            if 'ADD' in self.data:
                #Busca si ya ha sido insertado
                if(self.conectado == False):
                    self.conectado = True					
                    clientes['nombre'].append(self.data[4:])
                    clientes['socket'].append(self.conn)
                    for i in clientes['socket']:
                        if i != self.conn:
                            i.send(self.data[4:]+" ha entrado en el chat.")
                else:
                    self.conn.send("Ya estás en el chat.")
                print (clientes)
    
            if ('LIST' in self.data) and (self.conectado == True):
                for i in clientes['nombre']:
                        print ("enviado a "+str(self.conn)+" "+str(i))
                        self.conn.send(i+" ")
                print (clientes)
    
            if ('END' in self.data):
                if (self.conectado == True):
                    for i in clientes['socket']:
                        if i == self.conn:
                            nombre = clientes['nombre'][clientes['socket'].index(i)]
                            clientes['nombre'].remove(nombre)
                            clientes['socket'].remove(i)
                            self.conectado = False
                    for i in clientes['socket']:
                        if i != self.conn:		
                            i.send(nombre+" ha salido del chat.")
                    self.conn.send(" ")
                else:
                    self.conn.send(" ")
            if ('TEXT' in self.data and 'TEXT TO' not in self.data) and (self.conectado == True):
                for i in clientes['socket']:
                    if i != self.conn:
                        i.send(clientes['nombre'][clientes['socket'].index(self.conn)]+" dice: "+self.data[5:])
                print (clientes)
    
            if ('TEXT TO' in self.data) and (self.conectado == True):
                palabras = self.data[8:].split()
                #busca si existe alguien con ese alias
                for i in clientes['nombre']:
                    if i == palabras[0]:
                        #Prepara el mensaje
                        del palabras[0]
                        mensaje = string.join(palabras, ' ')
                        clientes['socket'][clientes['nombre'].index(i)].send(clientes['nombre'][clientes['socket'].index(self.conn)]+" dice: "+mensaje)
        self.conn.close() 
 
#creamos socket pasivo y escuchamos en el puerto 9000
s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
s.bind( ( socket.gethostname(), 9000 ) )
s.listen( 5 )
while(True):
	conn, addr = s.accept()
	gestionaClientes(conn).start()