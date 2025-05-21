#pragma once

#include "ObjetoJuego.h"

using namespace System;

enum class TipoRecurso {
    HABILIDAD_HUMANA_EMPATIA, HABILIDAD_HUMANA_ETICA, HABILIDAD_HUMANA_ADAPTABILIDAD, HABILIDAD_HUMANA_CREATIVIDAD,
    TECNOLOGIA_IA, TECNOLOGIA_BIG_DATA, TECNOLOGIA_ENERGIA, TECNOLOGIA_AUTOMATIZACION,
    BLOQUE_CONSTRUIDO,
    INDEFINIDO
};

ref class Recurso : public ObjetoJuego {
private:
    TipoRecurso tipo;
    String^ nombre;
    int valor;

public:
    Recurso(int xInicial, int yInicial, ConsoleColor col, TipoRecurso t, String^ nom, int val, String^ rep1, String^ rep2);

    virtual ~Recurso() override {}
    virtual void Dibujar() override;
    virtual void Limpiar() override;
    virtual void Actualizar(Mapa^ mapa, Jugador^ jugador) override;
    virtual bool EnColision(ObjetoJuego^ otro) override;

    TipoRecurso ObtenerTipoRecurso();
    String^ ObtenerNombre();
    int ObtenerValor();
    String^ ObtenerMiniAscii();
    ConsoleColor ObtenerColorIcono();
};