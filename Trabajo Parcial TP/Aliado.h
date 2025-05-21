#pragma once

#include "ObjetoJuego.h"

using namespace System;

ref class Mapa;
ref class Jugador;

ref class Aliado : public ObjetoJuego {
private:
    int velocidad;
    int contadorMovimiento;
public:
    Aliado(int xInicial, int yInicial, ConsoleColor colPorDefecto, int vel, wchar_t caracterTipo);
    virtual ~Aliado() override {}
    virtual void Dibujar() override;
    virtual void Limpiar() override;
    virtual void Actualizar(Mapa^ mapa, Jugador^ jugador) override;
    virtual bool EnColision(ObjetoJuego^ otro) override;
};