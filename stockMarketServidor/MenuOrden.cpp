#include "MenuOrden.h"
#include "Orden.h"
#include <vector>
#include <iostream>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <string>
#include <cstring>
#include "sqlite3.h"

bool actualizarSaldo(const std::string& email, double cantidad, bool sumar) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc = sqlite3_open("JP.sqlite", &db);
    if (rc) return false;

    const char* sqlSelect = "SELECT Dinero FROM Usuario WHERE Email = ?";
    rc = sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    double saldoActual = 0;
    if (rc == SQLITE_ROW) {
        saldoActual = sqlite3_column_double(stmt, 0);
    } else {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }
    sqlite3_finalize(stmt);

    double nuevoSaldo = sumar ? saldoActual + cantidad : saldoActual - cantidad;
    const char* sqlUpdate = "UPDATE Usuario SET Dinero = ? WHERE Email = ?";
    rc = sqlite3_prepare_v2(db, sqlUpdate, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_double(stmt, 1, nuevoSaldo);
    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return rc == SQLITE_DONE;
}

bool restarDineroAlComprador(const std::string& email, double total) {
    return actualizarSaldo(email, total, false);
}

bool sumarDineroAlVendedor(const std::string& email, double total) {
    return actualizarSaldo(email, total, true);
}

MenuOrden::MenuOrden(SOCKET comm_socket, const Usuario& usuario)
    : comm_socket(comm_socket), usuario(usuario) {}

std::string obtenerFechaActual() {
    time_t now = time(nullptr);
    tm* tstruct = localtime(&now);
    char buf[80];
    if (tstruct) {
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tstruct);
        return std::string(buf);
    }
    return "";
}

std::string obtenerEmailPorIdOrden(int idOrden) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    std::string email = "";
    if (sqlite3_open("JP.sqlite", &db) != SQLITE_OK) return email;

    const char* sql = "SELECT Email FROM Orden WHERE ID_Orden = ?";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, idOrden);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
    return email;
}

void MenuOrden::cancelarOrdenPendiente() {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        send(comm_socket, "Error al conectar con la base de datos.\n", 41, 0);
        return;
    }

    std::string consulta = "SELECT ID_Orden, TipoOrden, Ticker, Cantidad, Precio, Fecha_Creacion "
                           "FROM Orden WHERE Email = ? AND Estado = 0";
    rc = sqlite3_prepare_v2(db, consulta.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        send(comm_socket, "Error al preparar la consulta.\n", 32, 0);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, usuario.getEmail().c_str(), -1, SQLITE_STATIC);
    std::string resultado = "\n--- Órdenes Pendientes ---\n";
    std::vector<int> idsDisponibles;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        std::string tipo = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string ticker = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int cantidad = sqlite3_column_int(stmt, 3);
        double precio = sqlite3_column_double(stmt, 4);
        std::string fecha = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

        resultado += "ID: " + std::to_string(id) + ", Tipo: " + tipo + ", Ticker: " + ticker +
                     ", Cantidad: " + std::to_string(cantidad) + ", Precio: " + std::to_string(precio) +
                     ", Fecha: " + fecha + "\n";
        idsDisponibles.push_back(id);
    }

    sqlite3_finalize(stmt);

    if (idsDisponibles.empty()) {
        resultado += "No hay órdenes pendientes.\n";
        send(comm_socket, resultado.c_str(), resultado.size(), 0);
        sqlite3_close(db);
        return;
    }

    send(comm_socket, resultado.c_str(), resultado.size(), 0);
    send(comm_socket, "\nIngrese el ID de la orden a cancelar: ", 39, 0);

    char buffer[256];
    int bytes = recv(comm_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        sqlite3_close(db);
        return;
    }
    buffer[bytes] = '\0';
    int idIngresado = std::stoi(buffer);

    if (std::find(idsDisponibles.begin(), idsDisponibles.end(), idIngresado) == idsDisponibles.end()) {
        send(comm_socket, "ID no válido o no corresponde a una orden pendiente.\n", 54, 0);
        sqlite3_close(db);
        return;
    }

    std::string deleteSQL = "DELETE FROM Orden WHERE ID_Orden = ?";
    rc = sqlite3_prepare_v2(db, deleteSQL.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, idIngresado);
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            send(comm_socket, "Orden cancelada exitosamente.\n", 31, 0);
        } else {
            send(comm_socket, "Error al cancelar la orden.\n", 29, 0);
        }
        sqlite3_finalize(stmt);
    } else {
        send(comm_socket, "Error al preparar eliminación.\n", 31, 0);
    }

    sqlite3_close(db);
}
