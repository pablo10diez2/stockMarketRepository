#include "MenuCuenta.h"
#include "sqlite3.h"
#include "logs.h"
#include <iostream>
#include <string>
#include <winsock2.h>
#include <algorithm>
#include <sstream>

MenuCuenta::MenuCuenta(SOCKET socket, const std::string& email) : comm_socket(socket), email_usuario(email) {
    std::cout << "MenuCuenta creado para usuario: " << email << std::endl;
}

bool MenuCuenta::mostrarMenu() {
    char recvBuff[512];
    int bytes;
    bool menuActivo = true;
    bool salirCompleto = false;

    try {
        escribirLog("Usuario " + email_usuario + " accedió al menú de cuenta");
    } catch (const std::exception& e) {
        std::cout << "Error al escribir log: " << e.what() << std::endl;
    }

    while (menuActivo) {
        std::string menuOpciones = "\n=== MENU CUENTA ===\n1) Ver perfil\n2) Cambiar contrasena\n3) Introducir fondos\n4) Ver acciones\n5) Balance\n6) Volver\n7) Salir\nSeleccione una opción: ";
        send(comm_socket, menuOpciones.c_str(), menuOpciones.size(), 0);
        Sleep(100);

        bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
        if (bytes <= 0) return true;
        recvBuff[bytes] = '\0';
        std::string opcion(recvBuff);
        opcion.erase(std::remove(opcion.begin(), opcion.end(), '\r'), opcion.end());
        opcion.erase(std::remove(opcion.begin(), opcion.end(), '\n'), opcion.end());

        if (opcion == "1") {
            verPerfil();
        } else if (opcion == "2") {
            cambiarContrasena();
        } else if (opcion == "3") {
            introducirFondos();
        } else if (opcion == "4") {
            mostrarAccionesUsuario();
        } else if (opcion == "5") {
            mostrarBalance();
        } else if (opcion == "6") {
            std::string msg = "Volviendo al menú principal...\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            Sleep(100);
            menuActivo = false;
        } else if (opcion == "7") {
            std::string msg = "Saliendo del sistema. ¡Hasta pronto!\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            Sleep(100);
            menuActivo = false;
            salirCompleto = true;
        } else {
            std::string msg = "Opción inválida. Intente de nuevo.\n";
            send(comm_socket, msg.c_str(), msg.size(), 0);
            Sleep(100);
        }
    }
    return salirCompleto;
}

void MenuCuenta::mostrarBalance() {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;
    double ventas = 0.0, compras = 0.0, retenido = 0.0, saldoTotal = 0.0;

    rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        std::string errorMsg = "Error al conectar con la base de datos.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    const char* sqlVentas = "SELECT SUM(Precio * Cantidad) FROM Orden WHERE Email = ? AND TipoOrden = 'Venta' AND Estado = 1";
    rc = sqlite3_prepare_v2(db, sqlVentas, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, email_usuario.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
            ventas = sqlite3_column_double(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    const char* sqlCompras = "SELECT SUM(Precio * Cantidad) FROM Orden WHERE Email = ? AND TipoOrden = 'Compra' AND Estado = 1";
    rc = sqlite3_prepare_v2(db, sqlCompras, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, email_usuario.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
            compras = sqlite3_column_double(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    const char* sqlRetenido = "SELECT SUM(Precio * Cantidad) FROM Orden WHERE Email = ? AND TipoOrden = 'Compra' AND Estado = 0";
    rc = sqlite3_prepare_v2(db, sqlRetenido, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, email_usuario.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
            retenido = sqlite3_column_double(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    const char* sqlDinero = "SELECT Dinero FROM Usuario WHERE Email = ?";
    rc = sqlite3_prepare_v2(db, sqlDinero, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, email_usuario.c_str(), -1, SQLITE_STATIC);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            saldoTotal = sqlite3_column_double(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    sqlite3_close(db);

    double disponible = saldoTotal - retenido;
    double balance = ventas - compras;

    std::ostringstream resumen;
    resumen << "\n- Ingresos (Entradas de dinero): \n";
    resumen << "Ventas realizadas: " << ventas << "€ \n";
    resumen << "\n- Gastos (Salidas de dinero): \n";
    resumen << "Compras realizadas: " << compras << "€ \n";
    resumen << "\n- Resumen del balance: \n";
    resumen << "Saldo disponible: " << disponible << "€ \n";
    resumen << "Saldo retenido: " << retenido << "€ \n";
    resumen << "Saldo total: " << saldoTotal << "€ \n";
    resumen << "Balance total: " << balance << "€ \n";

    std::string textoFinal = resumen.str();
    send(comm_socket, textoFinal.c_str(), textoFinal.size(), 0);
    Sleep(100);
}


void MenuCuenta::verPerfil() {
    std::cout << "Ejecutando verPerfil() para usuario: " << email_usuario << std::endl;

    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        std::string errorMsg = "Error al conectar con la base de datos.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    std::string perfil = "\n=== PERFIL DE USUARIO ===\n";


    const char* sqlUser = "SELECT Nombre_Usuario, Apellido_Usuario, Email, ID_Rol, Dinero FROM Usuario WHERE Email = ?";
    rc = sqlite3_prepare_v2(db, sqlUser, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "Error al preparar la consulta de usuario.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, email_usuario.c_str(), -1, SQLITE_STATIC);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string nombre = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string apellido = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int rol = sqlite3_column_int(stmt, 3);
        double dineroTotal = sqlite3_column_double(stmt, 4);
        sqlite3_finalize(stmt);


        double retenido = 0.0;
        const char* sqlRetenido = "SELECT SUM(Precio * Cantidad) FROM Orden WHERE Email = ? AND TipoOrden = 'Compra' AND Estado = 0";
        if (sqlite3_prepare_v2(db, sqlRetenido, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, email_usuario.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
                retenido = sqlite3_column_double(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }

        double disponible = dineroTotal - retenido;

        perfil += "Nombre: " + nombre + "\n";
        perfil += "Apellido: " + apellido + "\n";
        perfil += "Email: " + email + "\n";
        perfil += "Rol: " + std::string((rol == 1) ? "Administrador" : "Usuario") + "\n";
        perfil += "Saldo disponible: " + std::to_string(disponible) + " €\n";
        perfil += "Saldo retenido: " + std::to_string(retenido) + " €\n";
    } else {
        std::string errorMsg = "No se encontró información del usuario.\n";
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        sqlite3_close(db);
        return;
    }


    perfil += "\n=== ACCIONES EN POSESIÓN ===\n";
    const char* sqlAcciones = R"(
        SELECT T1.Ticker,
               IFNULL(T1.Total - IFNULL(T2.EnVenta, 0), 0) AS Disponibles
        FROM (
            SELECT Ticker, SUM(CASE WHEN TipoOrden = 'Compra' THEN Cantidad ELSE -Cantidad END) AS Total
            FROM Orden
            WHERE Email = ? AND Estado = 1
            GROUP BY Ticker
        ) AS T1
        LEFT JOIN (
            SELECT Ticker, SUM(Cantidad) AS EnVenta
            FROM Orden
            WHERE Email = ? AND TipoOrden = 'Venta' AND Estado = 0
            GROUP BY Ticker
        ) AS T2
        ON T1.Ticker = T2.Ticker
        WHERE Disponibles > 0
    )";

    if (sqlite3_prepare_v2(db, sqlAcciones, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, email_usuario.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, email_usuario.c_str(), -1, SQLITE_STATIC);
        bool tieneAcciones = false;
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            std::string ticker = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            int cantidad = sqlite3_column_int(stmt, 1);
            perfil += "Ticker: " + ticker + " | Cantidad: " + std::to_string(cantidad) + "\n";
            tieneAcciones = true;
        }
        if (!tieneAcciones) {
            perfil += "No tienes acciones disponibles.\n";
        }
        sqlite3_finalize(stmt);
    } else {
        perfil += "Error al consultar las acciones.\n";
    }

    sqlite3_close(db);
    send(comm_socket, perfil.c_str(), perfil.size(), 0);
    Sleep(100);

    try {
        escribirLog("Usuario " + email_usuario + " consultó su perfil");
    } catch (const std::exception& e) {
        std::cout << "Error al escribir log: " << e.what() << std::endl;
    }
}


void MenuCuenta::cambiarContrasena() {
    std::cout << "Ejecutando cambiarContrasena() para usuario: " << email_usuario << std::endl;

    char recvBuff[512];
    int bytes;

    std::string msg = "Introduzca la nueva contraseña: ";
    std::cout << "Enviando: " << msg;
    send(comm_socket, msg.c_str(), msg.size(), 0);
    Sleep(100);

    std::cout << "Esperando nueva contraseña del cliente..." << std::endl;
    bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
    if (bytes <= 0) {
        std::cout << "Cliente desconectado durante cambio de contraseña" << std::endl;
        return;
    }
    recvBuff[bytes] = '\0';
    std::string nuevaContrasena(recvBuff);

    nuevaContrasena.erase(std::remove(nuevaContrasena.begin(), nuevaContrasena.end(), '\r'), nuevaContrasena.end());
    nuevaContrasena.erase(std::remove(nuevaContrasena.begin(), nuevaContrasena.end(), '\n'), nuevaContrasena.end());

    std::cout << "Nueva contraseña recibida, actualizando en BD..." << std::endl;

    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;

    rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        std::string errorMsg = "Error al conectar con la base de datos.\n";
        std::cout << "Error de SQLite: " << sqlite3_errmsg(db) << std::endl;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    const char* sql = "UPDATE Usuario SET Contrasena = ? WHERE Email = ?";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "Error al preparar la consulta.\n";
        std::cout << "Error de SQLite: " << sqlite3_errmsg(db) << std::endl;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, nuevaContrasena.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email_usuario.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    if (rc == SQLITE_DONE) {
        std::string exito = "Contraseña actualizada correctamente.\n";
        std::cout << "Enviando: " << exito;
        send(comm_socket, exito.c_str(), exito.size(), 0);
        Sleep(100);

        try {
            escribirLog("Usuario " + email_usuario + " cambió su contraseña");
        } catch (const std::exception& e) {
            std::cout << "Error al escribir log: " << e.what() << std::endl;
        }
    } else {
        std::string errorMsg = "Error al actualizar la contraseña.\n";
        std::cout << "Error de SQLite: " << sqlite3_errmsg(db) << std::endl;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void MenuCuenta::introducirFondos() {
    std::cout << "Ejecutando introducirFondos() para usuario: " << email_usuario << std::endl;

    char recvBuff[512];
    int bytes;

    std::string msg = "Introduce tu número de cuenta: ";
    std::cout << "Enviando: " << msg;
    send(comm_socket, msg.c_str(), msg.size(), 0);
    Sleep(100);

    std::cout << "Esperando número de cuenta del cliente..." << std::endl;
    bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
    if (bytes <= 0) {
        std::cout << "Cliente desconectado durante introducción de fondos" << std::endl;
        return;
    }
    recvBuff[bytes] = '\0';
    std::string numeroCuenta(recvBuff);
    numeroCuenta.erase(std::remove(numeroCuenta.begin(), numeroCuenta.end(), '\r'), numeroCuenta.end());
    numeroCuenta.erase(std::remove(numeroCuenta.begin(), numeroCuenta.end(), '\n'), numeroCuenta.end());

    std::cout << "Número de cuenta recibido: " << numeroCuenta << std::endl;

    msg = "Introduce la cantidad de fondos a añadir: ";
    std::cout << "Enviando: " << msg;
    send(comm_socket, msg.c_str(), msg.size(), 0);
    Sleep(100);

    std::cout << "Esperando cantidad de fondos del cliente..." << std::endl;
    bytes = recv(comm_socket, recvBuff, sizeof(recvBuff) - 1, 0);
    if (bytes <= 0) {
        std::cout << "Cliente desconectado durante introducción de fondos" << std::endl;
        return;
    }
    recvBuff[bytes] = '\0';
    std::string cantidadStr(recvBuff);
    cantidadStr.erase(std::remove(cantidadStr.begin(), cantidadStr.end(), '\r'), cantidadStr.end());
    cantidadStr.erase(std::remove(cantidadStr.begin(), cantidadStr.end(), '\n'), cantidadStr.end());

    std::cout << "Cantidad de fondos recibida: " << cantidadStr << std::endl;


    double cantidad = 0;
    try {
        cantidad = std::stod(cantidadStr);
        if (cantidad <= 0) {
            std::string errorMsg = "La cantidad debe ser un número positivo.\n";
            std::cout << "Enviando: " << errorMsg;
            send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
            return;
        }
    } catch (const std::exception& e) {
        std::string errorMsg = "Por favor, introduzca un valor numérico válido.\n";
        std::cout << "Enviando: " << errorMsg;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;

    rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        std::string errorMsg = "Error al conectar con la base de datos.\n";
        std::cout << "Error de SQLite: " << sqlite3_errmsg(db) << std::endl;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    const char* sqlSelect = "SELECT Dinero FROM Usuario WHERE Email = ?";
    rc = sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "Error al preparar la consulta.\n";
        std::cout << "Error de SQLite: " << sqlite3_errmsg(db) << std::endl;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, email_usuario.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);

    double dineroActual = 0;
    if (rc == SQLITE_ROW) {
        dineroActual = sqlite3_column_double(stmt, 0);
        std::cout << "Dinero actual del usuario: " << dineroActual << std::endl;
    } else {
        std::string errorMsg = "No se pudo obtener la información de la cuenta.\n";
        std::cout << "Error de SQLite: " << sqlite3_errmsg(db) << std::endl;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }

    sqlite3_finalize(stmt);


    double nuevoSaldo = dineroActual + cantidad;
    const char* sqlUpdate = "UPDATE Usuario SET Dinero = ? WHERE Email = ?";

    rc = sqlite3_prepare_v2(db, sqlUpdate, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "Error al preparar la actualización.\n";
        std::cout << "Error de SQLite: " << sqlite3_errmsg(db) << std::endl;
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
        std::cout << "Enviando: " << confirmacion;
        send(comm_socket, confirmacion.c_str(), confirmacion.size(), 0);
        Sleep(100);
        try {
            escribirLog("Usuario " + email_usuario + " añadió " + std::to_string(cantidad) + " € a su cuenta");
        } catch (const std::exception& e) {
            std::cout << "Error al escribir log: " << e.what() << std::endl;
        }
    } else {
        std::string errorMsg = "Error al actualizar el saldo.\n";
        std::cout << "Error de SQLite: " << sqlite3_errmsg(db) << std::endl;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void MenuCuenta::mostrarAccionesUsuario() {
    std::cout << "Ejecutando mostrarAccionesUsuario() para: " << email_usuario << std::endl;
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;

    rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        std::string errorMsg = "Error al abrir la base de datos.\n";
        std::cout << errorMsg;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    const char* sql = R"(
        SELECT Ticker, 
               SUM(CASE WHEN TipoOrden = 'Compra' THEN Cantidad ELSE -Cantidad END) AS Acciones
        FROM Orden
        WHERE Email = ? AND Estado = 1
        GROUP BY Ticker
        HAVING Acciones > 0;
    )";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::string errorMsg = "Error al preparar la consulta.\n";
        std::cout << errorMsg << " SQLite: " << sqlite3_errmsg(db) << std::endl;
        send(comm_socket, errorMsg.c_str(), errorMsg.size(), 0);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, email_usuario.c_str(), -1, SQLITE_STATIC);

    std::string resumen = "\n=== ACCIONES EN POSESIÓN ===\n";
    bool tieneAcciones = false;

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        const char* ticker = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        int cantidad = sqlite3_column_double(stmt, 1);
        resumen += "Ticker: " + std::string(ticker) + " | Cantidad: " + std::to_string(cantidad) + "\n";
        tieneAcciones = true;
    }

    if (!tieneAcciones) {
        resumen += "No tienes acciones en posesión.\n";
    }

    send(comm_socket, resumen.c_str(), resumen.size(), 0);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

