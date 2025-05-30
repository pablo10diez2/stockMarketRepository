#include "Accion.h"

Accion::Accion() {
    this->ticker = "";
    this->nombre = "";
    this->mercado = "";
}

Accion::Accion(const std::string& ticker, const std::string& nombre, const std::string& mercado) {
    this->ticker = ticker;
    this->nombre = nombre;
    this->mercado = mercado;
}

Accion::~Accion() {

}

void Accion::setTicker(const std::string& ticker) {
    this->ticker = ticker;
}

void Accion::setNombre(const std::string& nombre) {
    this->nombre = nombre;
}

void Accion::setMercado(const std::string& mercado) {
    this->mercado = mercado;
}

std::string Accion::getTicker() const{
    return this->ticker;
}

std::string Accion::getNombre() const{
    return this->nombre;
}

std::string Accion::getMercado() const {
    return this->mercado;
}
