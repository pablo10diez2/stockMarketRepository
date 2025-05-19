/*
 * funcionesBD.h
 *
 *  Created on: 17 may 2025
 *      Author: pablo.diez
 */

#ifndef FUNCIONESBD_H_
#define FUNCIONESBD_H_

#include "sqlite3.h"
#include "Usuario.h"

bool iniciarSesion(const std::string& email, const std::string& password);
bool registrarUsuario(const std::string& nombre, const std::string& apellido, const std::string& email, const std::string& password, int id_rol);
Usuario cargarUsuarioDesdeBD(const std::string& email);
#endif /* FUNCIONESBD_H_ */
