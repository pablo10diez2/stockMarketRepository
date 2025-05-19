#include "Usuario.h"

Usuario::Usuario() {
	this->nombre = "";
	this->apellido = "";
	this->email = "";
	this->contrasena = "";
	this->idRol = 0;
	this->dinero = 0.0;
}

Usuario::Usuario(const std::string& nombre, const std::string& apellido, const std::string& email,
			const std::string& contrasena, int idRol, double dinero) {
	this->nombre = nombre;
	this->apellido = apellido;
	this->email = email;
	this->contrasena = contrasena;
	this->idRol = idRol;
	this->dinero = dinero;
}
Usuario::~Usuario(){

}

void Usuario::setNombre(const std::string& nombre) {
	this->nombre = nombre;
}

void Usuario::setApellido(const std::string& apellido) {
	this->apellido = apellido;
}

void Usuario::setEmail(const std::string& email) {
	this->email = email;
}

void Usuario::setContrasena(const std::string& contrasena) {
	this->contrasena = contrasena;
}

void Usuario::setIdRol(int idRol) {
	this->idRol = idRol;
}

void Usuario::setDinero(double dinero) {
	this->dinero = dinero;
}

std::string Usuario::getNombre()  {
	return this->nombre;
}

std::string Usuario::getApellido(){
	return this->apellido;
}

std::string Usuario::getEmail(){
	return this->email;
}

std::string Usuario::getContrasena(){
	return this->contrasena;
}

int Usuario::getIdRol(){
	return this->idRol;
}

double Usuario::getDinero(){
	return this->dinero;
}
