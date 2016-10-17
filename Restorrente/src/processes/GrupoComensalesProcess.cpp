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
		vector<Semaforo*>* semsMesasLibres, vector<MemoriaCompartida<bool>*>* shmMesasLibres,
		Pipe* pipeLlamadosAMozos, vector<Semaforo*>* semsLlegoComida, vector<Semaforo*>* semsComidaEnMesas,
		vector<MemoriaCompartida<Comida>*>* shmComidaEnMesas, vector<Semaforo*>* semsMesaPago, Menu* menu) {

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

	this->semsComidaEnMesas = semsComidaEnMesas;
	this->shmComidaEnMesas = shmComidaEnMesas;

	this->menu = menu;

	inicializarMemoriasCompartidas();
}

void GrupoComensalesProcess::inicializarMemoriasCompartidas(){
	this->shmPersonasLiving->crear(SHM_PERSONAS_LIVING, 0);

	for (unsigned int i = 0; i < shmMesasLibres->size(); i++){
		shmMesasLibres->at(i)->crear(SHM_MESAS_LIBRES, i);
	}

	for (unsigned int i = 0; i < shmComidaEnMesas->size(); i++){
		shmComidaEnMesas->at(i)->crear(SHM_COMIDA_MESAS, i);
	}
}

int GrupoComensalesProcess::obtenerNumeroMesa(){
	int mesa = -1;

	cout << getpid() << " " << "INFO: Grupo de comensales buscando mesa libre. " <<  endl;

	for (unsigned int i = 0; i < shmMesasLibres->size(); i++){

		cout << getpid() << " " << "DEBUG: Grupo de comensales esperando semsMesasLibres: " << i <<  endl;
		semsMesasLibres->at(i)->p();
		cout << getpid() << " " << "DEBUG: Grupo de comensales obtuvo semsMesasLibres: " << i <<  endl;
		cout << getpid() << " " << "DEBUG: shmMesasLibres size: " << shmMesasLibres->size() <<  endl;
		bool mesaLibre = shmMesasLibres->at(i)->leer();
		cout << getpid() << " " << "DEBUG: Grupo de comensales leyo de shmMesasLibres: " << mesaLibre <<  endl;
		if (mesaLibre){
			mesa = i;
			cout << getpid() << " " << "INFO: Mesa libre encontrada. Nro mesa: " << mesa <<  endl;

			cout << getpid() << " " << "INFO: Ocupando mesa nro: " << mesa <<  endl;
			shmMesasLibres->at(i)->escribir(false);

		}
		cout << getpid() << " " << "DEBUG: Grupo de comensales liberando semsMesasLibres: " << i <<  endl;
		semsMesasLibres->at(i)->v();

		if (mesaLibre){
			break;
		}
	}

	if (mesa < 0){
		cout << getpid() << " " << "ERROR: No se encontro la mesa libre" <<  endl;
	}
	return mesa;

}


Pedido GrupoComensalesProcess::generarPedido() {
	Pedido pedido(mesa);

	for (int i = 0; i < cantPersonas; i++){
		Plato plato = menu->getPlatoRandom();
		cout << getpid() << " " << "INFO: Grupo de comensales elige " << plato.getNombre() << endl;
		pedido.agregarPlato(plato);
	}

	return pedido;

}


void GrupoComensalesProcess::llegar(){

	cout << getpid() << " " << "INFO: Llega grupo de comensales de " << cantPersonas << " personas" <<  endl;
	semComensalesEnPuerta->v();
	cout << getpid() << " " << "INFO: Grupo de comensales esperando recepcionista libre" << endl;
	semRecepcionistasLibres->p();
	cout << getpid() << " " << "INFO: Grupo de comensales siendo atendido" << endl;
	sleep(TIEMPO_ANTENDIENDO);

	cout << getpid() << " " << "DEBUG: Esperando semaforo personas living " << endl;
	semPersonasLivingB->p();
	cout << getpid() << " " << "DEBUG: Leyendo personas living " << endl;
	int personasLiving = shmPersonasLiving->leer();
	cout << getpid() << " " << "DEBUG: Personas en living " << personasLiving << endl;
	cout << getpid() << " " << "DEBUG: Incrementando personas living " << endl;
	shmPersonasLiving->escribir(personasLiving + 1);
	semPersonasLivingB->v();

	semMesasLibres->p();
	cout << getpid() << " " << "INFO: Grupo de comensales llendo a la mesa" << endl;


	cout << getpid() << " " << "DEBUG: Esperando semaforo personas living " << endl;
	semPersonasLivingB->p();
	cout << getpid() << " " << "DEBUG: Leyendo personas living " << endl;
	personasLiving = shmPersonasLiving->leer();
	cout << getpid() << " " << "DEBUG: Personas en living " << personasLiving << endl;
	cout << getpid() << " " << "DEBUG: Decrementando personas living " << endl;
	shmPersonasLiving->escribir(personasLiving - 1);
	semPersonasLivingB->v();

	cout << getpid() << " " << "DEBUG: Liberando memoria personas living " << endl;

	mesa = obtenerNumeroMesa();
}

void GrupoComensalesProcess::comer(){
	bool seguirPidiendo = true;
	while(seguirPidiendo){
		cout << getpid() << " " << "INFO: Grupo de comensales eligiendo comida" << endl;

		Pedido pedido = generarPedido();
		string pedidoStr = LlamadoAMozoSerializer::serializar(pedido);

		cout << getpid() << " " << "INFO: Grupo de comensales pidiendo comida." << endl;
		cout << getpid() << " " << "DEBUG: Grupo de comensales escribiendo en pipeLlamadosAMozos: " << pedidoStr << endl;

		pipeLlamadosAMozos->escribir(static_cast<const void*>(pedidoStr.c_str()), pedidoStr.size());

		cout << getpid() << " " << "INFO: Grupo de comensales esperando comida." << endl;
		semsLlegoComida->at(mesa)->p();
		cout << getpid() << " " << "INFO: Llego comida a mesa nro " << mesa << endl;

		semsComidaEnMesas->at(mesa)->p();
		Comida comida = shmComidaEnMesas->at(mesa)->leer();

		cout << getpid() << " " << "INFO: Grupo de comensales empezando a comer" << endl;

		vector<Plato> platos = comida.getPlatos();

		for (unsigned int i = 0; i < platos.size(); i++){
			cout << getpid() << " " << "INFO: Comiendo " << platos[i].getNombre() << endl;
		}
		sleep(TIEMPO_COMER);

		cout << getpid() << " " << "INFO: Grupo de comensales termino de comer." << endl;

		seguirPidiendo = (RandomUtil::randomCeroUno() < PROBABILIDAD_IRSE);
	}
}

void GrupoComensalesProcess::irse(){

	//TODO pagar


	cout << getpid() << " " << "DEBUG: Grupo de comensales esperando semsMesasLibres " << mesa << endl;
	semsMesasLibres->at(mesa)->p();
	cout << getpid() << " " << "INFO: Grupo de comensales liberando la mesa nro " << mesa << endl;
	shmMesasLibres->at(mesa)->escribir(true);
	semsMesasLibres->at(mesa)->v();

	cout << getpid() << " " << "INFO: Grupo de comensales se va de la mesa" << endl;
	semMesasLibres->v();

}


void GrupoComensalesProcess::run(){
	cout << "DEBUG: Iniciando grupo de comensales con pid: " << getpid() << endl;
	llegar();

//	comer();
	cout << getpid() << " " << "INFO: Grupo de comensales comiendo" << endl;
	sleep(TIEMPO_COMER);


	irse();
}

void GrupoComensalesProcess::liberarMemoriasCompartidas(){
	this->shmPersonasLiving->liberar();

	for (unsigned int i = 0; i < shmMesasLibres->size(); i++){
		shmMesasLibres->at(i)->liberar();
	}

	for (unsigned int i = 0; i < shmComidaEnMesas->size(); i++){
		shmComidaEnMesas->at(i)->liberar();
	}
}

GrupoComensalesProcess::~GrupoComensalesProcess() {
	liberarMemoriasCompartidas();
}

} /* namespace std */

