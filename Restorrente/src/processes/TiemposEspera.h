/*
 * TiemposEspera.h
 *
 *  Created on: Oct 20, 2016
 *      Author: gaston
 */

#ifndef PROCESSES_TIEMPOSESPERA_H_
#define PROCESSES_TIEMPOSESPERA_H_

using namespace std;

class TiemposEspera {

public:
	static const int TIEMPO_RANDOM_ENTRE_COMENSALES = 5;
	static const int TIEMPO_ATENDER = 2;
	static const int TIEMPO_TOMAR_PEDIDO = 3;
	static const int TIEMPO_COMER = 5;
	static const int TIEMPO_ELECCION_COMIDA = 2;
	static constexpr double PROBABILIDAD_DEJAR_DE_COMER = 0.25;
	static const int TIEMPO_COCINA = 1;
	static const int TIEMPO_CORTE_LUZ = 30;

};

#endif /* PROCESSES_TIEMPOSESPERA_H_ */
