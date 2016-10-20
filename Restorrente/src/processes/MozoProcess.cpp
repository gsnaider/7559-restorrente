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
	inicializarHandler();

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

	Logger::log(mozoLogId, "Mozo esperando para leer tamanio de pedido", DEBUG);
	do{
		Logger::log(mozoLogId, "Mozo esperando para leer pipeLlamadosAMozos", DEBUG);
		pipeLlamadosAMozos->leer(static_cast<void*>(&tamanioPedidoChar),sizeof(char));
		Logger::log(mozoLogId, "Mozo leyo " + Logger::intToString(tamanioPedidoChar) + " de pipeLlamadosAMozos", DEBUG);
		finLectura = (tamanioPedidoChar == LlamadoAMozoSerializer::SEPARADOR);
		if (!finLectura){
			tamanioPedidoStr += tamanioPedidoChar;
		}

	}while (!finLectura);

	return stoi(tamanioPedidoStr);
}

string MozoProcess::leerLlamado(int tamanioLlamado){
	 char buffer[tamanioLlamado];
	 string llamado;

	Logger::log(mozoLogId, "Mozo esperando para leer pipeLlamadosAMozos", DEBUG);

	pipeLlamadosAMozos->leer(static_cast<void*>(buffer),tamanioLlamado);

	Logger::log(mozoLogId, "Mozo termino de leerpipLlamadosAMozos", DEBUG);

	llamado = buffer;
	Logger::log(mozoLogId, "Mozo leyo " + llamado + " de pipeLlamadosAMozos", DEBUG);

	return llamado;
}


void MozoProcess::procesarPedido(Pedido pedido) {
	Logger::log(mozoLogId, "Mozo tomando pedido de mesa " + Logger::intToString(pedido.getMesa()), INFO);

	for (unsigned int i = 0; i < pedido.getPlatos().size(); i++){
		Logger::log(mozoLogId, "Mozo tomando pedido de " + pedido.getPlatos().at(i).getNombre(), INFO);
		sleep(TiemposEspera::TIEMPO_TOMAR_PEDIDO);
	}

	string pedidoStr = LlamadoAMozoSerializer::serializar(pedido);

	Logger::log(mozoLogId, "Mozo escribiendo en pipePedidosACocinar: " + pedidoStr, DEBUG);
	pipePedidosACocinar->escribir(static_cast<const void*>(pedidoStr.c_str()), pedidoStr.size());

	Logger::log(mozoLogId, "Mozo mando el pedido de la mesa " + Logger::intToString(pedido.getMesa()) + " a la cocina.", INFO);

}

void MozoProcess::procesarComida(Comida comida) {
	Logger::log(mozoLogId, "Mozo recibiendo comida para mesa " + Logger::intToString(comida.getMesa()), INFO);

	Logger::log(mozoLogId, "Mozo llevando comida a mesa " + Logger::intToString(comida.getMesa()), INFO);


	Logger::log(mozoLogId, "Mozo dejando comida en mesa " + Logger::intToString(comida.getMesa()), INFO);

	semsLlegoComida->at(comida.getMesa()).v();

}

void MozoProcess::procesarPedidoCuenta(PedidoCuenta pedidoCuenta) {

	int mesa = pedidoCuenta.getMesa();

	Logger::log(mozoLogId, "Mozo recibiendo cuenta de mesa " + Logger::intToString(mesa), INFO);


	Logger::log(mozoLogId, "Mozo esperando semsFacturas[" + Logger::intToString(mesa) + "]", DEBUG);

	semsFacturas->at(mesa).p();
	Logger::log(mozoLogId, "Mozo obtuvo semsFacturas[" + Logger::intToString(mesa) + "]", DEBUG);

	Logger::log(mozoLogId, "Mozo leyendo shmFacturas[" + Logger::intToString(mesa) + "]", DEBUG);

	double factura = shmFacturas->at(mesa).leer();
	Logger::log(mozoLogId, "Mozo leyo " + Logger::doubleToString(factura) + " de shmFacturas[" + Logger::intToString(mesa) + "]", DEBUG);

	Logger::log(mozoLogId, "Mesa " + Logger::intToString(mesa) + ": pagando factura: $" +  Logger::doubleToString(factura), INFO);


	Logger::log(mozoLogId, "Mozo esperando semCajaB", DEBUG);

	semCajaB->p();
	Logger::log(mozoLogId, "Mozo escribiendo 0 en shmFacturas[" + Logger::intToString(mesa) + "]", DEBUG);

	shmFacturas->at(mesa).escribir(0);

	Logger::log(mozoLogId, "Mozo leyendo shmCaja" , DEBUG);

	double cajaActual = shmCaja->leer();
	Logger::log(mozoLogId, "Mozo leyo " + Logger::doubleToString(cajaActual) + " de shmCaja", DEBUG);


	Logger::log(mozoLogId, "Mozo sumando $" + Logger::doubleToString(factura) + " a la caja", INFO);

	double cajaActualizada = cajaActual + factura;

	shmCaja->escribir(cajaActualizada);

	Logger::log(mozoLogId, "Valor actual de caja: $" + Logger::doubleToString(cajaActualizada), INFO);


	semCajaB->v();
	semsFacturas->at(mesa).v();
	semsMesaPago->at(mesa).v();

	Logger::log(mozoLogId, "Mozo libero semCajaB semsFacturas[" + Logger::intToString(mesa) + "] y semsMesasPago[" + Logger::intToString(mesa) + "]" , DEBUG);


}

void MozoProcess::run(){
	Logger::log(mozoLogId, "Iniciando mozo" , DEBUG);


	//TODO Ver si hay mejor forma que while(true).
	while(true){

		Logger::log(mozoLogId, "Mozo esperando a recibir llamados" , INFO);

		Logger::log(mozoLogId, "Mozo esperando semLlamadosAMozos" , DEBUG);

		semLlamadosAMozos->p();

		Logger::log(mozoLogId, "Mozo obtuvo semLlamadosAMozos" , DEBUG);

		int tamanioLlamado = leerTamanioLlamado();

		Logger::log(mozoLogId, "Mozo leyo tamanio de pedido: " + Logger::intToString(tamanioLlamado) , DEBUG);

		Logger::log(mozoLogId, "Mozo recibio un llamado" , INFO);

		string llamado = leerLlamado(tamanioLlamado);

		int tipoLlamado = LlamadoAMozoSerializer::getTipoLlamado(llamado);

		Logger::log(mozoLogId, "Mozo recibio tipo de llamado: " + Logger::intToString(tipoLlamado) , DEBUG);

		switch (tipoLlamado) {
			case LlamadoAMozoSerializer::PEDIDO: {
				Logger::log(mozoLogId, "Mozo recibio un pedido de una mesa" , INFO);

				Pedido pedido = LlamadoAMozoSerializer::deserializarPedido(llamado);
				procesarPedido(pedido);
				break;
			}
			case LlamadoAMozoSerializer::COMIDA: {

				Logger::log(mozoLogId, "Mozo recibio comida para llevar a una mesa" , INFO);

				Comida comida = LlamadoAMozoSerializer::deserializarComida(llamado);
				procesarComida(comida);
				break;
			}
			case LlamadoAMozoSerializer::PEDIDO_CUENTA: {

				Logger::log(mozoLogId, "Mozo recibio un pedido de cuenta" , INFO);


				PedidoCuenta pedidoCuenta = LlamadoAMozoSerializer::deserializarPedidoCuenta(llamado);
				procesarPedidoCuenta(pedidoCuenta);
				break;
			}
		}

		semLlamadosAMozos->v();

	}
}

void MozoProcess::limpiarRecursos(){
	Logger::log(mozoLogId, "Limpiando recursos", DEBUG);
	liberarMemoriasCompartidas();
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
