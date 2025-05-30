#ifndef USUARIO_H_
#define USUARIO_H_

#include <string>

class Usuario {
private:
	std::string nombre;
	std::string apellido;
	std::string email;
	std::string contrasena;
	int idRol;
	double dinero;

public:

	Usuario();
	Usuario(const std::string& nombre, const std::string& apellido, const std::string& email,
			const std::string& contrasena, int idRol, double dinero);
	~Usuario();

	void setNombre(const std::string& nombre);
	void setApellido(const std::string& apellido);
	void setEmail(const std::string& email);
	void setContrasena(const std::string& contrasena);
	void setIdRol(int idRol);
	void setDinero(double dinero);

	std::string getNombre();
	std::string getApellido();
	std::string getEmail();
	std::string getContrasena();
	int getIdRol();
	double getDinero();

};

#endif /* USUARIO_H_ */
