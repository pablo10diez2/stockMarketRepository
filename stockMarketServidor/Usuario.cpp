
#include "Usuario.h"
#include <string.h>

Usuario::Usuario(const char* nombre, const char* apellido, const char* email,
			const char* contrasena, int idRol, double dinero) {
	strcpy(this->nombre, nombre);
	strcpy(this->apellido, apellido);
	strcpy(this->email, email);
	strcpy(this->contrasena, contrasena);
	this->idRol = idRol;
	this->dinero = dinero;
}
Usuario::~Usuario(){

}

void Usuario::setNombre(const char* nombre) {
	strcpy(this->nombre, nombre);
}

void Usuario::setApellido(const char* apellido) {
	strcpy(this->apellido, apellido);
}

void Usuario::setEmail(const char* email) {
	strcpy(this->email, email);
}

void Usuario::setContrasena(const char* contrasena) {
	strcpy(this->contrasena, contrasena);
}

void Usuario::setIdRol(int idRol) {
	this->idRol = idRol;
}

void Usuario::setDinero(double dinero) {
	this->dinero = dinero;
}

const char* Usuario::getNombre()  {
	return this->nombre;
}

const char* Usuario::getApellido(){
	return this->apellido;
}

const char* Usuario::getEmail(){
	return this->email;
}

const char* Usuario::getContrasena(){
	return this->contrasena;
}

int Usuario::getIdRol(){
	return this->idRol;
}

double Usuario::getDinero(){
	return this->dinero;
}
