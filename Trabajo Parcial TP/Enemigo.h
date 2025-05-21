#pragma once

#include "ObjetoJuego.h"

using namespace System;

ref class Mapa;
ref class Jugador;

ref class Enemigo : public ObjetoJuego {
private:
    int dano;
    int velocidad;
    int contadorMovimiento;
    void MoverHaciaJugador(Mapa^ mapa, Jugador^ jugador, int% salidaDx, int% salidaDy);
    void MoverAleatorio(Mapa^ mapa, int% salidaDx, int% salidaDy);

public:
    Enemigo(int xInicial, int yInicial, ConsoleColor colPorDefecto, int dmg, int vel, wchar_t caracterTipo);
    virtual ~Enemigo() override {}
    virtual void Dibujar() override;
    virtual void Limpiar() override;
    virtual void Actualizar(Mapa^ mapa, Jugador^ jugador) override;
    virtual bool EnColision(ObjetoJuego^ otro) override;
    int ObtenerDano();
};