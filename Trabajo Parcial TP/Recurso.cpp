#include "Recurso.h"
#include "Mapa.h"   
#include "Jugador.h"
#include <cmath>   // Para Math::Min

using namespace System;

Recurso::Recurso(int xInicial, int yInicial, ConsoleColor col, TipoRecurso t, String^ nom, int val, String^ rep1, String^ rep2)
    : ObjetoJuego(xInicial, yInicial, col),
    tipo(t), nombre(nom), valor(val) {
    if (!String::IsNullOrEmpty(rep1)) AgregarLineaRepresentacion(rep1);
    if (!String::IsNullOrEmpty(rep2) && lineasRepresentacion < MAX_LINEAS_REPRESENTACION) AgregarLineaRepresentacion(rep2);

    int caracteresVisibles = 0;
    for (int i = 0; i < lineasRepresentacion; ++i) {
        if (representacion[i] != nullptr) caracteresVisibles += representacion[i]->Trim()->Length;
    }

    if (tipo != TipoRecurso::BLOQUE_CONSTRUIDO && caracteresVisibles < 2) {
        lineasRepresentacion = 0;
        for (int i = 0; i < MAX_LINEAS_REPRESENTACION; ++i) representacion[i] = nullptr;

        switch (t) {
        case TipoRecurso::HABILIDAD_HUMANA_EMPATIA:
        case TipoRecurso::HABILIDAD_HUMANA_ETICA:
        case TipoRecurso::HABILIDAD_HUMANA_ADAPTABILIDAD:
        case TipoRecurso::HABILIDAD_HUMANA_CREATIVIDAD:
            AgregarLineaRepresentacion("Hab");
            AgregarLineaRepresentacion("(*)");
            break;
        case TipoRecurso::TECNOLOGIA_IA:
        case TipoRecurso::TECNOLOGIA_BIG_DATA:
        case TipoRecurso::TECNOLOGIA_ENERGIA:
        case TipoRecurso::TECNOLOGIA_AUTOMATIZACION:
            AgregarLineaRepresentacion("Tec");
            AgregarLineaRepresentacion("[#]");
            break;
        default: AgregarLineaRepresentacion("R??"); break;
        }
    }
    else if (tipo == TipoRecurso::BLOQUE_CONSTRUIDO && lineasRepresentacion == 0) {
        AgregarLineaRepresentacion("[B]");
    }
    else if (lineasRepresentacion == 0) {
        AgregarLineaRepresentacion("RSC");
    }
}

void Recurso::Dibujar() {
    Console::ForegroundColor = color;
    int yActual = ObtenerYInicialDibujo();
    int xDibujo = ObtenerXInicialDibujo();

    for (int i = 0; i < lineasRepresentacion; ++i) {
        String^ linea = representacion[i];
        if (linea == nullptr) continue;
        try {
            if (yActual >= 0 && yActual < Console::BufferHeight &&
                xDibujo >= 0 && xDibujo + linea->Length <= Console::BufferWidth) {
                Console::SetCursorPosition(xDibujo, yActual);
                Console::Write(linea);
            }
            yActual++;
        }
        catch (ArgumentOutOfRangeException^) { break; }
    }
    Console::ResetColor();
}

void Recurso::Limpiar() {
    int yActual = yPrev;
    int anchoMaximoArte = 0;
    for (int i = 0; i < lineasRepresentacion; ++i) {
        if (representacion[i] != nullptr && representacion[i]->Length > anchoMaximoArte) {
            anchoMaximoArte = representacion[i]->Length;
        }
    }
    int xLimpiar = xPrev - anchoMaximoArte / 2;

    for (int i = 0; i < lineasRepresentacion; ++i) {
        String^ linea = representacion[i];
        if (linea == nullptr) continue;
        try {
            if (yActual >= 0 && yActual < Console::BufferHeight &&
                xLimpiar >= 0 && xLimpiar + linea->Length <= Console::BufferWidth) {
                Console::SetCursorPosition(xLimpiar, yActual);
                Console::Write(gcnew String(L' ', linea->Length));
            }
            yActual++;
        }
        catch (ArgumentOutOfRangeException^) { break; }
    }
}

void Recurso::Actualizar(Mapa^ mapa, Jugador^ jugador) {
    // Estático
}

bool Recurso::EnColision(ObjetoJuego^ otro) {
    return false;
}

TipoRecurso Recurso::ObtenerTipoRecurso() { return tipo; }
String^ Recurso::ObtenerNombre() { return nombre; }
int Recurso::ObtenerValor() { return valor; }

String^ Recurso::ObtenerMiniAscii() {
    if (lineasRepresentacion > 0 && representacion[0] != nullptr && !String::IsNullOrWhiteSpace(representacion[0])) {
        String^ primeraLineaTrimmed = representacion[0]->Trim();
        if (primeraLineaTrimmed->Length == 0 && lineasRepresentacion > 1 && representacion[1] != nullptr && !String::IsNullOrWhiteSpace(representacion[1])) {
            String^ segundaLineaTrimmed = representacion[1]->Trim();
            return segundaLineaTrimmed->Substring(0, Math::Min(segundaLineaTrimmed->Length, 2));
        }
        return primeraLineaTrimmed->Substring(0, Math::Min(primeraLineaTrimmed->Length, 2));
    }
    else if (lineasRepresentacion > 1 && representacion[1] != nullptr && !String::IsNullOrWhiteSpace(representacion[1])) {
        String^ segundaLineaTrimmed = representacion[1]->Trim();
        return segundaLineaTrimmed->Substring(0, Math::Min(segundaLineaTrimmed->Length, 2));
    }

    switch (tipo) {
    case TipoRecurso::HABILIDAD_HUMANA_EMPATIA: return "Em";
    case TipoRecurso::HABILIDAD_HUMANA_ETICA: return "Et";
    case TipoRecurso::HABILIDAD_HUMANA_ADAPTABILIDAD: return "Ad";
    case TipoRecurso::HABILIDAD_HUMANA_CREATIVIDAD: return "Cr";
    case TipoRecurso::TECNOLOGIA_IA: return "IA";
    case TipoRecurso::TECNOLOGIA_BIG_DATA: return "BD";
    case TipoRecurso::TECNOLOGIA_ENERGIA: return "En";
    case TipoRecurso::TECNOLOGIA_AUTOMATIZACION: return "Au";
    case TipoRecurso::BLOQUE_CONSTRUIDO: return "[]";
    default: return "??";
    }
}

ConsoleColor Recurso::ObtenerColorIcono() {
    return this->color;
}