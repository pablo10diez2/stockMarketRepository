

#ifndef PROGRAMA_H_
#define PROGRAMA_H_

#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include "funcionesBD.h"
#include "MenuCuenta.h"
#include "logs.h"
#include "sqlite3.h"
#include "Usuario.h"
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6000

bool mostrarMenuPrincipal(SOCKET comm_socket, Usuario usuario);

#endif
