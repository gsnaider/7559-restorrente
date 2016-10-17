/*
 * CocineroProcess.cpp
 *
 *  Created on: Oct 15, 2016
 *      Author: gaston
 */

#include "CocineroProcess.h"

#include <unistd.h>
#include <iostream>
#include <string>

#include "../main/MainProcess.h"
#include "../model/Plato.h"
#include "../utils/serializer/LlamadoAMozoSerializer.h"

using namespace std;

CocineroProcess::CocineroProcess(Pipe* pipePedidosACocinar,
		vector<Semaforo>* semsFacturas,
		vector<MemoriaCompartida<double>>* shmFacturas,
		Pipe* pipeLlamadosAMozos) {

	this->pipePedidosACocinar = pipePedidosACocinar;

	this->semsFacturas = semsFacturas;
	this->shmFacturas = shmFacturas;

	this->pipeLlamadosAMozos = pipeLlamadosAMozos;

	inicializarMemoriasCompartidas();

}

void CocineroProcess::inicializarMemoriasCompartidas() {

	for (unsigned int i = 0; i < shmFacturas->size(); i++){
		shmFacturas->at(i).crear(SHM_FACTURAS, i);
	}

}

int CocineroProcess::leerTamanioPedido(){
	char tamanioPedidoChar;
	string tamanioPedidoStr;
	bool finLectura = false;

	cout << getpid() << " " << "DEBUG: Cocinero esperando para leer tamanio de pedido"<< endl;
	do{
		cout << getpid() << " " << "DEBUG: Cocinero esperando para leer pipePedidosACocinar"<< endl;

		pipePedidosACocinar->leer(static_cast<void*>(&tamanioPedidoChar),sizeof(char));

		cout << getpid() << " " << "DEBUG: Cocinero leyo " << tamanioPedidoChar << " de pipePedidosACocinar"<< endl;

		finLectura = (tamanioPedidoChar == LlamadoAMozoSerializer::SEPARADOR);
		if (!finLectura){
			tamanioPedidoStr += tamanioPedidoChar;
		}

	}while (!finLectura);

	return stoi(tamanioPedidoStr);
}

string CocineroProcess::leerPedido(int tamanioPedido) {

	char buffer[tamanioPedido];
	string pedido;

	cout << getpid() << " " << "DEBUG: Cocinero esperando para leer pipePedidosACocinar"<< endl;

	pipePedidosACocinar->leer(static_cast<void*>(buffer),tamanioPedido);

	cout << getpid() << " " << "DEBUG: Cocinero termino de leer pipePedidosACocinar"<< endl;

	pedido = buffer;
	cout << getpid() << " " << "DEBUG: Cocinero leyo " << pedido << " de pipePedidosACocinar"<< endl;

	return pedido;
}

Comida CocineroProcess::cocinar(Pedido pedido) {
	cout << getpid() << " " << "INFO: Cocinero recibio un pedido de mesa " << pedido.getMesa() << endl;

	Comida comida(pedido.getMesa());

	for (unsigned int i = 0; i < pedido.getPlatos().size(); i++){
		cout << getpid() << " " << "INFO: Cocinero cocinando " << pedido.getPlatos().at(i).getNombre() << endl;

		sleep(TIEMPO_COCINA);

		cout << getpid() << " " << "INFO: Cocinero termino de cocinar " << pedido.getPlatos().at(i).getNombre() << endl;

		comida.agregarPlato(pedido.getPlatos().at(i));

		//TODO Agregar precio de plato a factura de mesa.

	}


	cout << getpid() << " " << "INFO: Cocinero termino de cocinar pedido de mesa " << pedido.getMesa() << endl;

	return comida;
}


void CocineroProcess::run(){
	cout << getpid() << " " << "DEBUG: Iniciando cocinero"<< endl;

	//TODO Ver si hay mejor forma que while(true).
	while(true){

		cout << getpid() << " " << "INFO: Cocinero esperando a recibir pedidos"<< endl;

		int tamanioPedido = leerTamanioPedido();

		cout << getpid() << " " << "DEBUG: Cocinero leyo tamanio de pedido: " << tamanioPedido << endl;

		cout << getpid() << " " << "INFO: Cocinero recibio un pedido"<< endl;

		string pedidoStr = leerPedido(tamanioPedido);
		Pedido pedido = LlamadoAMozoSerializer::deserializarPedido(pedidoStr);

		Comida comida = cocinar(pedido);

		string comidaStr = LlamadoAMozoSerializer::serializar(comida);


		cout << getpid() << " " << "DEBUG: Cocinero escribiendo en pipeLlamadosAMozos: " << comidaStr << endl;

		pipeLlamadosAMozos->escribir(static_cast<const void*>(comidaStr.c_str()), comidaStr.size());

		cout << getpid() << " " << "INFO: Cocinero: ya esta lista la comida para llevar a la mesa " << comida.getMesa() << endl;

	}
}



CocineroProcess::~CocineroProcess() {
	// TODO Auto-generated destructor stub
}




