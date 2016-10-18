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
	Logger::log(cocineroLogId, "Cocinero esperando para leer tamanio de pedido", DEBUG);
	do{
		Logger::log(cocineroLogId, "Cocinero esperando para leer pipePedidosACocinar", DEBUG);

		pipePedidosACocinar->leer(static_cast<void*>(&tamanioPedidoChar),sizeof(char));

		Logger::log(cocineroLogId, "Cocinero leyo " + Logger::intToString(tamanioPedidoChar) + " de pipePedidosACocinar", DEBUG);

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

	Logger::log(cocineroLogId, "Cocinero esperando para leer pipePedidosACocinar", DEBUG);

	pipePedidosACocinar->leer(static_cast<void*>(buffer),tamanioPedido);

	Logger::log(cocineroLogId, "Cocinero termino de leer pipePedidosACocinar", DEBUG);

	pedido = buffer;
	Logger::log(cocineroLogId, "Cocinero leyo" + pedido + " de pipePedidoACocinar", DEBUG);


	return pedido;
}

void CocineroProcess::facturar(int mesa, Plato plato){
	double precioPlato = plato.getPrecio();

	Logger::log(cocineroLogId, "Cocinero esperando semsFacturas[" + Logger::intToString(mesa) + "]", DEBUG);

	semsFacturas->at(mesa).p();
	Logger::log(cocineroLogId, "Cocinero obtuvo semsFacturas[" + Logger::intToString(mesa) + "]", DEBUG);

	Logger::log(cocineroLogId, "Cocinero leyendo shmFacturas[" + Logger::intToString(mesa) + "]", DEBUG);

	double facturaActual = shmFacturas->at(mesa).leer();
	Logger::log(cocineroLogId, "Cocinero leyo " + Logger::doubleToString(mesa) + " de shmFacturas[" + Logger::intToString(mesa) + "]", DEBUG);

	Logger::log(cocineroLogId, "Cocinero sumando $" + Logger::doubleToString(precioPlato) + " a cuenta de mesa " + Logger::intToString(mesa), DEBUG);
	double facturaActualizada = facturaActual + precioPlato;

	Logger::log(cocineroLogId, "Cocinero escribiendo shmFacturas[" + Logger::intToString(mesa) + "]", DEBUG);

	shmFacturas->at(mesa).escribir(facturaActualizada);

	Logger::log(cocineroLogId, "Factura actual de mesa " + Logger::intToString(mesa) + ": $" + Logger::doubleToString(facturaActualizada), INFO);

	Logger::log(cocineroLogId, "Cocinero escribio en shmFacturas[" + Logger::intToString(mesa) + "]", DEBUG);

	semsFacturas->at(mesa).v();
}

Comida CocineroProcess::cocinar(Pedido pedido) {
	Logger::log(cocineroLogId, "Cocinero recibio un pedido de mesa " + Logger::intToString(pedido.getMesa()), INFO);

	Comida comida(pedido.getMesa());

	for (unsigned int i = 0; i < pedido.getPlatos().size(); i++){
		Logger::log(cocineroLogId, "Cocinero cocinando " + pedido.getPlatos().at(i).getNombre(), INFO);

		sleep(TIEMPO_COCINA);

		Logger::log(cocineroLogId, "Cocinero termino de cocinar " + pedido.getPlatos().at(i).getNombre(), INFO);
		comida.agregarPlato(pedido.getPlatos().at(i));

		facturar(pedido.getMesa(),pedido.getPlatos().at(i));

	}



	Logger::log(cocineroLogId, "Cocinero termino de cocinar pedido de mesa " + Logger::intToString(pedido.getMesa()), INFO);
	return comida;
}


void CocineroProcess::run(){
	Logger::log(cocineroLogId, "Iniciando cocinero " , DEBUG);

	//TODO Ver si hay mejor forma que while(true).
	while(true){

		Logger::log(cocineroLogId, "Cocinero esperando a recibir pedidos " , INFO);

		int tamanioPedido = leerTamanioPedido();

		Logger::log(cocineroLogId, "Cocinero leyo tamanio de pedido: " + Logger::intToString(tamanioPedido), DEBUG);

		Logger::log(cocineroLogId, "Cocinero recibio un pedido " , INFO);

		string pedidoStr = leerPedido(tamanioPedido);
		Pedido pedido = LlamadoAMozoSerializer::deserializarPedido(pedidoStr);

		Comida comida = cocinar(pedido);

		string comidaStr = LlamadoAMozoSerializer::serializar(comida);


		Logger::log(cocineroLogId, "Cocinero escribiendo en pipeLlamadosAMozos: " + comidaStr, DEBUG);

		pipeLlamadosAMozos->escribir(static_cast<const void*>(comidaStr.c_str()), comidaStr.size());

		Logger::log(cocineroLogId, "Cocinero: ya esta lista la comida para llevar a la mesa " + Logger::intToString(comida.getMesa()), INFO);

	}
}

void CocineroProcess::liberarMemoriasCompartidas(){

	for (unsigned int i = 0; i < shmFacturas->size(); i++){
		shmFacturas->at(i).liberar();
	}

}



CocineroProcess::~CocineroProcess() {
	// TODO Auto-generated destructor stub
}




