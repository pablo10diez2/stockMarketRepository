/*
 * funcionesBD.h
 *
 *  Created on: 17 may 2025
 *      Author: pablo.diez
 */

#ifndef FUNCIONESBD_H_
#define FUNCIONESBD_H_

#include "sqlite3.h"

bool iniciarSesion(const std::string& email, const std::string& password);

#endif /* FUNCIONESBD_H_ */
