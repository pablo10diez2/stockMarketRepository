/*
 * Usuario.h
 *
 *  Created on: 17 may 2025
 *      Author: pablo.diez
 */

#ifndef USUARIO_H_
#define USUARIO_H_

class Usuario {
private:
	char nombre[50];
	char apellido[50];
	char email[50];
	char contrasena[50];
	int idRol;
	double dinero;

public:
	Usuario(const char* nombre, const char* apellido, const char* email,
			const char* contrasena, int idRol, double dinero);
	~Usuario();

	void setNombre(const char* nombre);
	void setApellido(const char* apellido);
	void setEmail(const char* email);
	void setContrasena(const char* contrasena);
	void setIdRol(int idRol);
	void setDinero(double dinero);

	const char* getNombre();
	const char* getApellido();
	const char* getEmail();
	const char* getContrasena();
	int getIdRol();
	double getDinero();

};

#endif /* USUARIO_H_ */
