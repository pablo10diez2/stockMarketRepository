#ifndef MENUCUENTA_H_
#define MENUCUENTA_H_

#include <string>
#include <winsock2.h>

class MenuCuenta {
private:
    SOCKET comm_socket;
    std::string email_usuario;

    // Funciones internas para cada opción del menú
    void verPerfil();
    void cambiarContrasena();
    void introducirFondos();

public:
    MenuCuenta(SOCKET socket, const std::string& email);
    void mostrarMenu();
};

#endif /* MENUCUENTA_H_ */
