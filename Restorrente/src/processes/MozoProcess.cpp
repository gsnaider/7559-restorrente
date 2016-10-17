/*
 * MozoProcess.cpp
 *
 *  Created on: Oct 15, 2016
 *      Author: gaston
 */

#include "MozoProcess.h"

#include <unistd.h>
#include <iostream>
#include <string>

#include "../utils/serializer/LlamadoAMozoSerializer.h"

namespace std {

MozoProcess::MozoProcess(Pipe* pipeLlamadosAMozos, Pipe* pipePedidosACocinar, Semaforo* semLlamadosAMozos, vector<Semaforo>* semsLlegoComida,
		vector<Semaforo>* semsFacturas, vector<MemoriaCompartida<double>>* shmFacturas,
		Semaforo* semCajaB, MemoriaCompartida<double>* shmCaja, vector<Semaforo>* semsMesaPago) {

	this->pipeLlamadosAMozos = pipeLlamadosAMozos;
	this->pipePedidosACocinar = pipePedidosACocinar;
	this->semLlamadosAMozos = semLlamadosAMozos;
	this->semsLlegoComida = semsLlegoComida;
	this->semsFacturas = semsFacturas;
	this->shmFacturas = shmFacturas;
	this->semCajaB = semCajaB;
	this->shmCaja = shmCaja;
	this->semsMesaPago = semsMesaPago;

	inicializarMemoriasCompartidas();

}

void MozoProcess::inicializarMemoriasCompartidas(){
	this->shmCaja->crear(SHM_CAJA, 0);


	for (unsigned int i = 0; i < shmFacturas->size(); i++){
		shmFacturas->at(i).crear(SHM_FACTURAS, i);
	}
}

int MozoProcess::leerTamanioLlamado(){
	char tamanioPedidoChar;
	string tamanioPedidoStr;
	bool finLectura = false;

	cout << getpid() << " " << "DEBUG: Mozo esperando para leer tamanio de pedido"<< endl;
	do{
		cout << getpid() << " " << "DEBUG: Mozo esperando para leer pipeLlamadosAMozos"<< endl;
		pipeLlamadosAMozos->leer(static_cast<void*>(&tamanioPedidoChar),sizeof(char));
		cout << getpid() << " " << "DEBUG: Mozo leyo " << tamanioPedidoChar << " de pipeLlamadosAMozos"<< endl;
		finLectura = (tamanioPedidoChar == LlamadoAMozoSerializer::SEPARADOR);
		if (!finLectura){
			tamanioPedidoStr += tamanioPedidoChar;
		}

	}while (!finLectura);

	return stoi(tamanioPedidoStr);
}

string MozoProcess::leerLlamado(int tamanioLlamado){
	 char * buffer = new char [tamanioLlamado];
	 string llamado;

	cout << getpid() << " " << "DEBUG: Mozo esperando para leer pipeLlamadosAMozos"<< endl;

	pipeLlamadosAMozos->leer(static_cast<void*>(buffer),tamanioLlamado);

	cout << getpid() << " " << "DEBUG: Mozo termino de leer pipeLlamadosAMozos"<< endl;

	llamado = buffer;
	cout << getpid() << " " << "DEBUG: Mozo leyo " << llamado << " de pipeLlamadosAMozos"<< endl;

	return llamado;
}


void MozoProcess::procesarPedido(Pedido pedido) {
	cout << getpid() << " " << "INFO: Mozo tomando pedido de mesa " << pedido.getMesa() << endl;

	for (unsigned int i = 0; i < pedido.getPlatos().size(); i++){
		cout << getpid() << " " << "INFO: Mozo tomando pedido de " << pedido.getPlatos().at(i).getNombre() << endl;
	}

	string pedidoStr = LlamadoAMozoSerializer::serializar(pedido);

	cout << getpid() << " " << "DEBUG: Mozo escribiendo en pipePedidosACocinar: " << pedidoStr << endl;
	pipePedidosACocinar->escribir(static_cast<const void*>(pedidoStr.c_str()), pedidoStr.size());

	cout << getpid() << " " << "INFO: Mozo mando el pedido de la mesa " << pedido.getMesa() << " a la cocina." << endl;

}

void MozoProcess::procesarComida(Comida comida) {
	cout << getpid() << " " << "INFO: Mozo recibiendo comida para mesa " << comida.getMesa() << endl;


	cout << getpid() << " " << "INFO: Mozo llevando comida a mesa " << comida.getMesa() << endl;


	cout << getpid() << " " << "INFO: Mozo dejando comida en mesa " << comida.getMesa() << endl;
	semsLlegoComida->at(comida.getMesa()).v();

}

void MozoProcess::procesarPedidoCuenta(PedidoCuenta pedidoCuenta) {
	cout << getpid() << " " << "INFO: Mozo recibiendo cuenta de mesa " << pedidoCuenta.getMesa() << endl;
	sleep(5);

}

void MozoProcess::run(){
	cout << getpid() << " " << "DEBUG: Iniciando mozo"<< endl;

	//TODO Ver si hay mejor forma que while(true).
	while(true){

		cout << getpid() << " " << "INFO: Mozo esperando a recibir llamados"<< endl;

		cout << getpid() << " " << "DEBUG: Mozo esperando semLlamadosAMozos"<< endl;

		semLlamadosAMozos->p();

		cout << getpid() << " " << "DEBUG: Mozo obtuvo semLlamadosAMozos"<< endl;

		int tamanioLlamado = leerTamanioLlamado();

		cout << getpid() << " " << "DEBUG: Mozo leyo tamanio de pedido: " << tamanioLlamado << endl;

		cout << getpid() << " " << "INFO: Mozo recibio un llamado"<< endl;

		string llamado = leerLlamado(tamanioLlamado);

		int tipoLlamado = LlamadoAMozoSerializer::getTipoLlamado(llamado);

		cout << getpid() << " " << "DEBUG: Mozo recibio tipo de llamado: " << tipoLlamado << endl;

		switch (tipoLlamado) {
		case LlamadoAMozoSerializer::PEDIDO: {

			cout << getpid() << " " << "INFO: Mozo recibio un pedido de una mesa" << endl;

			Pedido pedido = LlamadoAMozoSerializer::deserializarPedido(llamado);
			procesarPedido(pedido);
			break;
		}
		case LlamadoAMozoSerializer::COMIDA: {

			cout << getpid() << " " << "INFO: Mozo recibio comida para llevar a una mesa" << endl;

			Comida comida = LlamadoAMozoSerializer::deserializarComida(llamado);
			procesarComida(comida);
			break;
		}
		case LlamadoAMozoSerializer::PEDIDO_CUENTA: {

			cout << getpid() << " " << "INFO: Mozo recibio un pedido de cuenta" << endl;

			PedidoCuenta pedidoCuenta = LlamadoAMozoSerializer::deserializarPedidoCuenta(llamado);
			procesarPedidoCuenta(pedidoCuenta);
			break;
		}
		}

		semLlamadosAMozos->v();



	}
}


void MozoProcess::liberarMemoriasCompartidas(){
	this->shmCaja->liberar();


	for (unsigned int i = 0; i < shmFacturas->size(); i++){
		shmFacturas->at(i).liberar();
	}
}



MozoProcess::~MozoProcess() {
	liberarMemoriasCompartidas();
}

} /* namespace std */
