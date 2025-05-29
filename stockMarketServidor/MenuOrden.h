#ifndef MENU_ORDEN_H
#define MENU_ORDEN_H

#include "Usuario.h"
#include <winsock2.h>

class MenuOrden {
private:
    SOCKET comm_socket;
    Usuario usuario;

    void mostrarOrdenesAnteriores();
    void comprar();
    void vender();

public:
    MenuOrden(SOCKET comm_socket, const Usuario& usuario);
    bool mostrarMenu(); // Devuelve true si el usuario quiere salir completamente
};

#endif
