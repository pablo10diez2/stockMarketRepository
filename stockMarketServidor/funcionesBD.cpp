#include <iostream>
#include <string>
#include "funcionesBD.h"

bool iniciarSesion(const std::string& email, const std::string& password) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;

    rc = sqlite3_open("usuarios.db", &db);
    if (rc) {
        std::cerr << "No se pudo abrir la base de datos: " << sqlite3_errmsg(db) << '\n';
        return false;
    }

    const char* sql = "SELECT * FROM Usuario WHERE Email = ? AND Contraseña = ?";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Fallo al preparar la consulta: " << sqlite3_errmsg(db) << '\n';
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    bool loginCorrecto = false;

    if (rc == SQLITE_ROW) {
        std::cout << "Inicio de sesión exitoso. Bienvenido, "
                  << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)) << "!\n";
        loginCorrecto = true;
    } else {
        std::cout << "Correo o contraseña incorrectos.\n";
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return loginCorrecto;
}

