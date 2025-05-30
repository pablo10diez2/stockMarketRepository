#include <iostream>
#include <string>
#include "funcionesBD.h"
#include "sqlite3.h"

#include <cctype>

std::string limpiarInput(const std::string& input) {
    size_t start = 0, end = input.size();

    while (start < end && (std::isspace((unsigned char)input[start]) || input[start] == '\r' || input[start] == '\n')) {
        ++start;
    }
    while (end > start && (std::isspace((unsigned char)input[end - 1]) || input[end - 1] == '\r' || input[end - 1] == '\n')) {
        --end;
    }

    return input.substr(start, end - start);
}


bool iniciarSesion(const std::string& email, const std::string& password) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;

    rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        std::cerr << "No se pudo abrir la base de datos: " << sqlite3_errmsg(db) << '\n';
        return false;
    }

    const char* sql = "SELECT * FROM Usuario WHERE Email = ? AND Contrasena = ?";

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

bool registrarUsuario(const std::string& nombre, const std::string& apellido,
                     const std::string& email, const std::string& password, int id_rol) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;

    rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        std::cerr << "No se pudo abrir la base de datos: " << sqlite3_errmsg(db) << '\n';
        return false;
    }


    const char* sqlCheck = "SELECT COUNT(*) FROM Usuario WHERE Email = ?";
    rc = sqlite3_prepare_v2(db, sqlCheck, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Fallo al preparar la consulta de verificación: " << sqlite3_errmsg(db) << '\n';
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0) {
        std::cout << "Error: El correo electrónico ya está registrado.\n";
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    sqlite3_finalize(stmt);


    const char* sqlInsert = "INSERT INTO Usuario (Nombre_Usuario, Apellido_Usuario, Email, Contrasena, ID_Rol, Dinero) "
                          "VALUES (?, ?, ?, ?, ?, 0)";

    rc = sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Fallo al preparar la consulta de inserción: " << sqlite3_errmsg(db) << '\n';
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(stmt, 1, nombre.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, apellido.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, email.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, password.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 5, id_rol);

    rc = sqlite3_step(stmt);
    bool registroCorrecto = false;

    if (rc == SQLITE_DONE) {
        std::cout << "Registro exitoso. ¡Bienvenido, " << nombre << "!\n";
        registroCorrecto = true;
    } else {
        std::cerr << "Error al registrar el usuario: " << sqlite3_errmsg(db) << '\n';
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return registroCorrecto;
}

Usuario cargarUsuarioDesdeBD(const std::string& email) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;

    Usuario usuario;

    rc = sqlite3_open("JP.sqlite", &db);
    if (rc) {
        std::cerr << "No se pudo abrir la base de datos: " << sqlite3_errmsg(db) << '\n';
        return usuario;
    }

    const char* sql = "SELECT Nombre_Usuario, Apellido_Usuario, Email, Contrasena, ID_Rol, Dinero "
                      "FROM Usuario WHERE Email = ?";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Fallo al preparar la consulta: " << sqlite3_errmsg(db) << '\n';
        sqlite3_close(db);
        return usuario;
    }

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) {
        std::string nombre = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string apellido = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string dbEmail = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        int id_rol = sqlite3_column_int(stmt, 4);
        double dinero = sqlite3_column_double(stmt, 5);


        usuario = Usuario(nombre, apellido, dbEmail, password, id_rol, dinero);
        std::cout << "Usuario cargado: " << nombre << " " << apellido << " (Email: " << dbEmail << ")\n";
    } else {
        std::cerr << "No se encontró al usuario con email: " << email << "\n";
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return usuario;
}
