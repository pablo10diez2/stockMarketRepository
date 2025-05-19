#include "MenuCuenta.h"
#include "sqlite3.h"
#include "logs.h"
#include <iostream>
#include <string>
#include <winsock2.h>

MenuCuenta::MenuCuenta(SOCKET socket, const std::string& email) : comm_socket(socket), email_usuario(email) {
}

bool MenuCuenta::mostrarMenu() {
    char recvBuff[512];
    int bytes;
    bool menuActivo = true;
    bool salirCompleto = false;

    while (menuActivo) {
        std::string menuOpciones = "\n=== MENU CUENTA ===\n1) Ver perfil\n2) Cambiar contrasena\n3) Introducir fondos\n4) Volver\n5) Salir\nSeleccione una opción: ";
        send(comm_socket, menuOpciones.c_str(), menuOpciones.size(), 0);

        bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
        if (bytes <= 0) {
            escribirLog("Error de comunicación en MenuCuenta para usuario: " + email_usuario);
            return false;
        }

        recvBuff[bytes] = '\0';
        std::string opcion(recvBuff);

        if (opcion == "1") {
            verPerfil();
        } else if (opcion == "2") {
            cambiarContrasena();
        } else if (opcion == "3") {
            introducirFondos();
        } else if (opcion == "4") {
            std::string msg = "Volviendo al menú principal...\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            menuActivo = false;
        } else if (opcion == "5") {
            std::string msg = "Saliendo del sistema. ¡Hasta pronto!\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            menuActivo = false;
            salirCompleto = true;
        } else {
            std::string msg = "Opción inválida. Intente de nuevo.\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
        }
    }

    return salirCompleto;
}

void MenuCuenta::verPerfil() {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;

    rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        std::string errorMsg = "Error al conectar con la base de datos.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    const char* sql = "SELECT Nombre_Usuario, Apellido_Usuario, Email, ID_Rol, Dinero "
                    "FROM Usuario WHERE Email = ?";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "Error al preparar la consulta.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, email_usuario.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        std::string nombre = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string apellido = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int rol = sqlite3_column_int(stmt, 3);
        double dinero = sqlite3_column_double(stmt, 4);

        std::string rolTexto = (rol == 1) ? "Administrador" : "Usuario";

        std::string perfil = "\n=== PERFIL DE USUARIO ===\n";
        perfil += "Nombre: " + nombre + "\n";
        perfil += "Apellido: " + apellido + "\n";
        perfil += "Email: " + email + "\n";
        perfil += "Rol: " + rolTexto + "\n";
        perfil += "Dinero disponible: " + std::to_string(dinero) + " €\n";

        send(comm_socket, perfil.c_str(), perfil.size(), 0);
        escribirLog("Usuario " + email_usuario + " consultó su perfil");
    } else {
        std::string errorMsg = "No se encontró información del usuario.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void MenuCuenta::cambiarContrasena() {
    char recvBuff[512];
    int bytes;

    std::string msg = "Introduzca la nueva contraseña: ";
    send(comm_socket, msg.c_str(), msg.size(), 0);

    bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
    if (bytes <= 0) return;
    recvBuff[bytes] = '\0';
    std::string nuevaContrasena(recvBuff);

    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;

    rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        std::string errorMsg = "Error al conectar con la base de datos.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    const char* sql = "UPDATE Usuario SET Contrasena = ? WHERE Email = ?";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "Error al preparar la consulta.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, nuevaContrasena.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email_usuario.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE) {
        std::string exito = "Contraseña actualizada correctamente.\n";
        send(comm_socket, exito.c_str(), exito.size(), 0);
        escribirLog("Usuario " + email_usuario + " cambió su contraseña");
    } else {
        std::string errorMsg = "Error al actualizar la contraseña.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void MenuCuenta::introducirFondos() {
    char recvBuff[512];
    int bytes;

    std::string msg = "Introduce tu número de cuenta: ";
    send(comm_socket, msg.c_str(), msg.size(), 0);

    bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
    if (bytes <= 0) return;
    recvBuff[bytes] = '\0';
    std::string numeroCuenta(recvBuff);

    msg = "Introduce la cantidad de fondos a añadir: ";
    send(comm_socket, msg.c_str(), msg.size(), 0);

    bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
    if (bytes <= 0) return;
    recvBuff[bytes] = '\0';

    // Convertir a número y validar
    double cantidad = 0;
    try {
        cantidad = std::stod(recvBuff);
        if (cantidad <= 0) {
            std::string errorMsg = "La cantidad debe ser un número positivo.\n";
            send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
            return;
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Por favor, introduzca un valor numérico válido.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;

    rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        std::string errorMsg = "Error al conectar con la base de datos.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    // Primero, obtener el dinero actual
    const char* sqlSelect = "SELECT Dinero FROM Usuario WHERE Email = ?";
    rc = sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "Error al preparar la consulta.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, email_usuario.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);

    double dineroActual = 0;
    if (rc == SQLITE_ROW) {
        dineroActual = sqlite3_column_double(stmt, 0);
    } else {
        std::string errorMsg = "No se pudo obtener la información de la cuenta.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }

    sqlite3_finalize(stmt);

    // Actualizar con el nuevo saldo
    double nuevoSaldo = dineroActual + cantidad;
    const char* sqlUpdate = "UPDATE Usuario SET Dinero = ? WHERE Email = ?";

    rc = sqlite3_prepare_v2(db, sqlUpdate, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "Error al preparar la actualización.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_double(stmt, 1, nuevoSaldo);
    sqlite3_bind_text(stmt, 2, email_usuario.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE) {
        std::string confirmacion = "Se han añadido " + std::to_string(cantidad) +
                                " € a tu cuenta.\nNuevo saldo: " +
                                std::to_string(nuevoSaldo) + " €\n";
        send(comm_socket, confirmacion.c_str(), confirmacion.size(), 0);
        escribirLog("Usuario " + email_usuario + " añadió " + std::to_string(cantidad) + " € a su cuenta");
    } else {
        std::string errorMsg = "Error al actualizar el saldo.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}
