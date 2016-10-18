/*

 * Parser.cpp
 *
 *  Created on: Oct 16, 2016
 *      Author: juampa_94


#include "Parser.h"


#include <iostream>
#include <string>

using namespace std;

Parser::Parser() {
}

Parser::~Parser() {
}

bool Parser::parse(string pathFile){
    ifstream data(pathFile);

    string line;
    getline(data,line);
    this->cantMozos = stoi(line);
    getline(data,line);
    this->cantRecepcionistas = stoi(line);
    getline(data,line);
    this->cantMesas = stoi(line);
    getline(data,line);
    this->cantComensales = stoi(line);
    return true;
 }

int Parser::getCantMozos(){
	return this->cantMozos;
}

int Parser::getCantRecepcionistas(){
	return this->cantRecepcionistas;
}
int Parser::getCantMesas(){
	return this->cantMesas;
}
int Parser::getCantComensales(){
	return this->cantComensales;
}
*/

#include "Parser.h"

Parser::Parser()
{

}

Parser::~Parser()
{

}


double string_to_double( const string& s )
{
	std::istringstream i(s);
	double x;
	i >> x;
	return x;
}

int string_to_int(const string& s){
	int result;
	std::stringstream(s) >> result;
	return result;
}

void Parser::extraerCantRecepcionistas(const pugi::xml_document* doc)
{
	pugi::xml_node setUpNode = doc->child("SetUp");

	std::string cantRecepcionistas = setUpNode.child("cantRecepcionistas").first_child().value();

	this->cantRecepcionistas = string_to_int(cantRecepcionistas);
}

void Parser::extraerCantComensales(const pugi::xml_document* doc)
{
	pugi::xml_node setUpNode = doc->child("SetUp");

	std::string cantComensales = setUpNode.child("cantComensales").first_child().value();

	this->cantComensales = string_to_int(cantComensales);

}
void Parser::extraerCantMesas(const pugi::xml_document* doc){

	pugi::xml_node setUpNode = doc->child("SetUp");
	std::string cantMesas = setUpNode.child("cantMesas").first_child().value();


	this->cantMesas = string_to_int(cantMesas);

}
void Parser::extraerCantMozos(const pugi::xml_document* doc){

	pugi::xml_node setUpNode = doc->child("SetUp");
	std::string cantMozos = setUpNode.child("cantMozos").first_child().value();

	this->cantMozos = string_to_int(cantMozos);

}
void Parser::extraerNivelDeLog(const pugi::xml_document* doc){

	pugi::xml_node setUpNode = doc->child("SetUp");
	std::string nivelDeLog = setUpNode.child("nivelDeLog").first_child().value();
	this->nivelDeLog = string_to_int(nivelDeLog);
}

void Parser::extraerMenu(const pugi::xml_document* doc){

	pugi::xml_node menuNode = doc->first_child().child("Menu");

	for (pugi::xml_node plate = menuNode.first_child(); plate; plate = plate.next_sibling())
	{
	   std::string nombre = plate.child("nombre").first_child().value();

	   std::string precio = plate.child("precio").first_child().value();


	   Plato newPlato(nombre,string_to_double(precio));
	   this->menu.agregarPlato(newPlato);
	}

}

void Parser::parsearDocumento(const std::string& nombreArchivoXML)
{
	pugi::xml_document doc;


	pugi::xml_parse_result result = doc.load_file(nombreArchivoXML.c_str());


	pugi::xml_node root = doc.root().first_child();

	extraerCantRecepcionistas(&doc);
	extraerCantMesas(&doc);
	extraerCantMozos(&doc);
	extraerCantComensales(&doc);
	extraerMenu(&doc);
	extraerNivelDeLog(&doc);
}

int Parser::getCantComensales(){
	return this->cantComensales;
}
int Parser::getCantRecepcionistas(){
	return this->cantRecepcionistas;
}
int Parser::getCantMesas(){
	return this->cantMesas;
}
int Parser::gentCantMozos(){
	return this->cantMozos;
}
Menu Parser::getMenu(){
	return this->menu;
}

int Parser::getNivelDeLog(){
	return this->nivelDeLog;
}

