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
   bool mostrarMenu();
   void cancelarOrdenPendiente();
   void restarDineroAlComprador(const std::string& email, double cantidad);
   void sumarDineroAlVendedor(const std::string& email, double cantidad);
   std::string obtenerEmailPorIdOrden(int idOrden);

};
#endif
