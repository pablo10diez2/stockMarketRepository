#ifndef ACCION_H
#define ACCION_H

#include <string>

class Accion {
private:
    std::string ticker;
    std::string nombre;
    std::string mercado;

public:
    Accion();
    Accion(const std::string& ticker, const std::string& nombre, const std::string& mercado);
    ~Accion();

    void setTicker(const std::string& ticker);
    void setNombre(const std::string& nombre);
    void setMercado(const std::string& mercado);

    std::string getTicker() const;
    std::string getNombre() const;
    std::string getMercado() const;
};

#endif
