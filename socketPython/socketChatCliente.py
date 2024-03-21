#!/usr/bin/python
# -*- coding: utf-8 -*-
#Cliente

from Tkinter import *
import threading
import socket

class App:
    def __init__(self, master, socket):
        self.conn = socket
        self.frame = Frame(master)
        self.label = Label(self.frame, text="Cliente chat. Python+TkInter.")
        self.textarea = Text(self.frame, height=20, width=40)
        self.scroll = Scrollbar(self.frame, command=self.textarea.yview)
        self.textarea.configure(yscrollcommand=self.scroll.set)

        self.texto_enviar = StringVar()
        self.text_ent = Entry(self.frame, textvariable=self.texto_enviar)
        
        self.btn_enviar = Button(self.frame, text="Enviar", command=self.enviar)
        self.btn_salir = Button(self.frame, text="Salir", command=self.salir)
        
        self.frame.grid()
        self.label.grid(row=0, column=0, columnspan=3)
        self.textarea.grid(row=1, column=0, columnspan=2)
        self.scroll.grid(row=1, column=2, sticky=N+S)
        self.text_ent.grid(row=2, column=0, columnspan=2, sticky=W+E)
        self.btn_enviar.grid(row=3, column=0)
        self.btn_salir.grid(row=3, column=1)
	    

    def enviar(self):
        self.conn.send(self.text_ent.get())
        self.text_ent.delete(0, END)

    # Esta es una función de retrollamada.
    def salir(self):
        lee.parar()
        self.conn.send("END")
        self.frame.quit()

    def escribir(self, texto):
	    self.textarea.insert(END, texto)

class leer(threading.Thread):
    def __init__(self, socket):
        threading.Thread.__init__(self)
        self.mensaje = ''
        self.conn = socket
        self.stop = False
    def run(self):
        while (self.stop == False):
            self.mensaje = self.conn.recv( 1024 )
            app.escribir(self.mensaje+'\n')
        self.conn.close()

    def parar(self):
	    self.stop = True

miSocket = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
miSocket.connect( (socket.gethostname(), 9000 ) )

lee = leer(miSocket)

lee.start()

root = Tk()
root.title('Chat Python-TkInter')
app = App(root, miSocket)

root.mainloop()