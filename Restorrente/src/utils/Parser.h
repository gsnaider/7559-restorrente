/*
 * Parser.h
 *
 *  Created on: Mar 23, 2016
 *      Author: gonzalo
 */

#ifndef UTILS_PARSER_PARSER_H_
#define UTILS_PARSER_PARSER_H_

#include <string>
#include <algorithm>
#include "pugixml/pugixml.hpp"
#include "../model/Menu.h"
#include <sstream>
#include <iostream>


class Parser
{
private:
	int cantRecepcionistas;
	int cantComensales;
	int cantMesas;
	int cantMozos;
	int nivelDeLog;
	Menu* menu;

	void extraerCantComensales(const pugi::xml_document* doc);
	void extraerCantRecepcionistas(const pugi::xml_document* doc);
	void extraerCantMesas(const pugi::xml_document* doc);
	void extraerCantMozos(const pugi::xml_document* doc);
	void extraerNivelDeLog(const pugi::xml_document* doc);
	void extraerMenu(const pugi::xml_document* doc);

public:
	Parser();
	~Parser();
	void parsearDocumento(const std::string& nombreArchivoXML);
	int getCantComensales();
	int getCantRecepcionistas();
	int getCantMesas();
	int gentCantMozos();
	int getNivelDeLog();
	Menu* getMenu();

};


#endif /* UTILS_PARSER_PARSER_H_ */
