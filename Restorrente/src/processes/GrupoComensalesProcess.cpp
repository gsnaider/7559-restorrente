/*
 * GrupoComensalesProcess.cpp
 *
 *  Created on: Oct 15, 2016
 *      Author: gaston
 */

#include "GrupoComensalesProcess.h"

#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>

#include "../model/Comida.h"
#include "../model/Pedido.h"
#include "../model/Plato.h"
#include "../utils/random/RandomUtil.h"
#include "../utils/serializer/LlamadoAMozoSerializer.h"

namespace std {

GrupoComensalesProcess::GrupoComensalesProcess(int cantPersonas, Semaforo* semRecepcionistasLibres, Semaforo* semComensalesEnPuerta,
		Semaforo* semPersonasLivingB, MemoriaCompartida<int>* shmPersonasLiving, Semaforo* semMesasLibres,
		vector<Semaforo>* semsMesasLibres, vector<MemoriaCompartida<bool>>* shmMesasLibres,
		Pipe* pipeLlamadosAMozos, vector<Semaforo>* semsLlegoComida, vector<Semaforo>* semsMesaPago, Menu menu) {

	this->mesa = -1; //Despues se setea el valor real.

	this->cantPersonas = cantPersonas;
	this->semComensalesEnPuerta = semComensalesEnPuerta;
	this->semRecepcionistasLibres = semRecepcionistasLibres;
	this->semPersonasLivingB = semPersonasLivingB;
	this->semMesasLibres = semMesasLibres;

	this->shmPersonasLiving = shmPersonasLiving;


	this->pipeLlamadosAMozos = pipeLlamadosAMozos;

	this->semsLlegoComida = semsLlegoComida;
	this->semsMesaPago = semsMesaPago;

	this->semsMesasLibres = semsMesasLibres;
	this->shmMesasLibres = shmMesasLibres;

	this->menu = menu;

	inicializarMemoriasCompartidas();
}

void GrupoComensalesProcess::inicializarMemoriasCompartidas(){
	this->shmPersonasLiving->crear(SHM_PERSONAS_LIVING, 0);

	for (unsigned int i = 0; i < shmMesasLibres->size(); i++){
		shmMesasLibres->at(i).crear(SHM_MESAS_LIBRES, i);
	}

}

int GrupoComensalesProcess::obtenerNumeroMesa(){
	int mesa = -1;


	Logger::log(comensalLogId, "Grupo de comensales buscando mesa libre. ", INFO);
	for (unsigned int i = 0; i < shmMesasLibres->size(); i++){

		Logger::log(comensalLogId, "Grupo de comensales esperando semsMesasLibre: " + Logger::intToString(i), DEBUG);

		semsMesasLibres->at(i).p();
		Logger::log(comensalLogId, "Grupo de comensales obtuvo semsMesasLibre: " + Logger::intToString(i), DEBUG);

		Logger::log(comensalLogId, "shmMesasLibre size: " + Logger::intToString(shmMesasLibres->size()), DEBUG);

		bool mesaLibre = shmMesasLibres->at(i).leer();
		//cout << getpid() << " " << "DEBUG: Grupo de comensales leyo de shmMesasLibres: " << mesaLibre <<  endl;
		//anda bien sumar string + bool? Calculo que si
		Logger::log(comensalLogId, "Grupo de comensales leyo de shmMesasLibres: " + mesaLibre, DEBUG);

		if (mesaLibre){
			mesa = i;
			Logger::log(comensalLogId, "Mesa libre encontrara. Nro mesa: " + Logger::intToString(mesa), INFO);

			Logger::log(comensalLogId, "Ocupando mesa nro: " + Logger::intToString(mesa), INFO);

			shmMesasLibres->at(i).escribir(false);

		}
		Logger::log(comensalLogId, "Grupo de comensales liberando semsMesasLibre: " + Logger::intToString(i), DEBUG);

		semsMesasLibres->at(i).v();

		if (mesaLibre){
			break;
		}
	}

	if (mesa < 0){
		Logger::log(comensalLogId, "No se encontro la mesa libre " , ERROR);

	}
	return mesa;

}


Pedido GrupoComensalesProcess::generarPedido() {
	Pedido pedido(mesa);

	for (int i = 0; i < cantPersonas; i++){
		Plato plato = menu.getPlatoRandom();
		Logger::log(comensalLogId, "Grupo de comensales elige " + plato.getNombre(), INFO);

		pedido.agregarPlato(plato);
	}

	return pedido;

}


void GrupoComensalesProcess::llegar(){
	Logger::log(comensalLogId, "Llega grupo de comensales de "+ Logger::intToString(cantPersonas) + " personas", INFO);

	semComensalesEnPuerta->v();
	Logger::log(comensalLogId, "Grupo de comensales esperando recepcionista libre ", INFO);

	semRecepcionistasLibres->p();
	Logger::log(comensalLogId, "Grupo de comensales siendo atendido por recepcionista ", INFO);

	sleep(TIEMPO_ANTENDIENDO);

	Logger::log(comensalLogId, "Esperando semaforo personas living ", DEBUG);

	semPersonasLivingB->p();
	Logger::log(comensalLogId, "Leyendo personas living ", DEBUG);

	int personasLiving = shmPersonasLiving->leer();
	Logger::log(comensalLogId, "Personas living " + Logger::intToString(personasLiving), DEBUG);
	Logger::log(comensalLogId, "Incrementando personas living ", DEBUG);

	shmPersonasLiving->escribir(personasLiving + 1);
	semPersonasLivingB->v();

	semMesasLibres->p();
	Logger::log(comensalLogId, "Grupo de comensales yendo a la mesa ", INFO);


	Logger::log(comensalLogId, "Esperando semaforo personas living ", DEBUG);

	semPersonasLivingB->p();
	Logger::log(comensalLogId, "Leyendo personas living ", DEBUG);
	personasLiving = shmPersonasLiving->leer();
	Logger::log(comensalLogId, "Ppersonas en living: " + Logger::intToString(personasLiving), DEBUG);
	Logger::log(comensalLogId, "Decrementando personas living ", DEBUG);
	shmPersonasLiving->escribir(personasLiving - 1);
	semPersonasLivingB->v();

	Logger::log(comensalLogId, "Liberando memoria personas living ", DEBUG);

	mesa = obtenerNumeroMesa();
}


void GrupoComensalesProcess::comer(){
	bool seguirPidiendo = true;
	int i = 0;

	while(seguirPidiendo){
		Logger::log(comensalLogId, "Grupo de comensales eligiendo comida", INFO);

		Pedido pedido = generarPedido();
		string pedidoStr = LlamadoAMozoSerializer::serializar(pedido);
		Logger::log(comensalLogId, "Grupo de comensales pidiendo comida", INFO);

		Logger::log(comensalLogId, "Grupo de comensales escribiendo en pipeLlamadosAMozos: " + pedidoStr, DEBUG);

		pipeLlamadosAMozos->escribir(static_cast<const void*>(pedidoStr.c_str()), pedidoStr.size());

		Logger::log(comensalLogId, "Grupo de comensales esperando comida", INFO);
		semsLlegoComida->at(mesa).p();
		Logger::log(comensalLogId, "Grupo de comensales: Llego comida a mesa nro " + Logger::intToString(mesa), INFO);

		Logger::log(comensalLogId, "Grupo de comensales empezando a comer", INFO);

		vector<Plato> platos = pedido.getPlatos();

		for (unsigned int i = 0; i < platos.size(); i++){
			Logger::log(comensalLogId, "Comiendo " + platos[i].getNombre(), INFO);

		}
		sleep(TIEMPO_COMER);

		Logger::log(comensalLogId, "Grupo de comensales termino de comer.", INFO);

		// TODO Arreglar, no esta funcionando el random.
		//seguirPidiendo = (RandomUtil::randomCeroUno() < PROBABILIDAD_IRSE);

		//TODO Solucion temporal (borrar).
		i++;
		seguirPidiendo = (i < 2);

	}
}

void GrupoComensalesProcess::pagar(){

	Logger::log(comensalLogId, "Grupo de comensales pidiendo la cuenta.", INFO);

	PedidoCuenta pedidoCuenta(mesa);
	string pedidoCuentaStr = LlamadoAMozoSerializer::serializar(pedidoCuenta);

	Logger::log(comensalLogId, "Grupo de comensales escribiendo en pipeLlamadosAMozos: " + pedidoCuentaStr, DEBUG);

	pipeLlamadosAMozos->escribir(static_cast<const void*>(pedidoCuentaStr.c_str()), pedidoCuentaStr.size());

	Logger::log(comensalLogId, "Grupo de comensales esperando para pagar.", INFO);

	semsMesaPago->at(mesa).p();

}

void GrupoComensalesProcess::irse(){


	Logger::log(comensalLogId, "Grupo de comensales esperando semsMesasLibres " + Logger::intToString(mesa), DEBUG);

	semsMesasLibres->at(mesa).p();

	Logger::log(comensalLogId, "Grupo de comensales liberando la mesa nro " + Logger::intToString(mesa), INFO);

	shmMesasLibres->at(mesa).escribir(true);
	semsMesasLibres->at(mesa).v();

	Logger::log(comensalLogId, "Grupo de comensales se va de la mesa", INFO);

	semMesasLibres->v();

}


void GrupoComensalesProcess::run(){
	Logger::log(comensalLogId, "Iniciando grupo de comensales comensal", DEBUG);

	llegar();
	comer();
	pagar();
	irse();
}

void GrupoComensalesProcess::liberarMemoriasCompartidas(){
	this->shmPersonasLiving->liberar();

	for (unsigned int i = 0; i < shmMesasLibres->size(); i++){
		shmMesasLibres->at(i).liberar();
	}

}

GrupoComensalesProcess::~GrupoComensalesProcess() {
	liberarMemoriasCompartidas();
}

} /* namespace std */

