#ifndef MENUCUENTA_H_
#define MENUCUENTA_H_

#include <string>
#include <winsock2.h>

class MenuCuenta {
private:
    SOCKET comm_socket;
    std::string email_usuario;

    void verPerfil();
    void cambiarContrasena();
    void introducirFondos();
    void mostrarAccionesUsuario();
    void mostrarBalance();

public:
    MenuCuenta(SOCKET socket, const std::string& email);
    bool mostrarMenu(); // Retorna true si el usuario quiere cerrar completamente
};

#endif /* MENUCUENTA_H_ */
