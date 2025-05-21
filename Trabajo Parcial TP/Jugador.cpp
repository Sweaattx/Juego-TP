#include "Jugador.h"
#include "Enemigo.h"
#include "Recurso.h"
#include "Aliado.h"
#include "Mapa.h"
#include <stdexcept>
#include <ctime>

using namespace System;

Jugador::Jugador(int xInicial, int yInicial, int vidasIniciales, int energiaInicial)
    : ObjetoJuego(xInicial, yInicial, ConsoleColor::Cyan),
    vidas(vidasIniciales), energia(energiaInicial), cantidadInventario(0),
    capacidadInventario(CAPACIDAD_INICIAL_INVENTARIO) {
    inventario = gcnew array<Recurso^>(capacidadInventario);
    tiempoUltimoDano = 0;
    AgregarLineaRepresentacion("(·_·)");
    AgregarLineaRepresentacion("/ T \\");
    AgregarLineaRepresentacion("^ ^ ^");
    int caracteresVisibles = 0;
    for (int i = 0; i < lineasRepresentacion; ++i) {
        if (representacion[i] != nullptr) caracteresVisibles += representacion[i]->Trim()->Length;
    }
    if (caracteresVisibles < 5) {
        lineasRepresentacion = 0;
        for (int i = 0; i < MAX_LINEAS_REPRESENTACION; ++i) representacion[i] = nullptr;
        AgregarLineaRepresentacion("  O  "); AgregarLineaRepresentacion(" /|\\ "); AgregarLineaRepresentacion(" / \\ ");
    }
}

Jugador::~Jugador() {}

void Jugador::Dibujar() {
    Console::ForegroundColor = color; int yActual = y; int anchoMaximoArte = 0;
    for (int i = 0; i < lineasRepresentacion; ++i) if (representacion[i] != nullptr && representacion[i]->Length > anchoMaximoArte) anchoMaximoArte = representacion[i]->Length;
    int xDibujo = x - anchoMaximoArte / 2;
    for (int i = 0; i < lineasRepresentacion; ++i) {
        String^ linea = representacion[i]; if (linea == nullptr) continue;
        try {
            if (yActual >= 0 && yActual < Console::BufferHeight && xDibujo >= 0 && xDibujo + linea->Length <= Console::BufferWidth) {
                Console::SetCursorPosition(xDibujo, yActual); Console::Write(linea);
            } yActual++;
        }
        catch (ArgumentOutOfRangeException^) { break; }
    } Console::ResetColor();
}

void Jugador::Limpiar() {
    int yActual = yPrev; int anchoMaximoArte = 0;
    for (int i = 0; i < lineasRepresentacion; ++i) if (representacion[i] != nullptr && representacion[i]->Length > anchoMaximoArte) anchoMaximoArte = representacion[i]->Length;
    int xLimpiar = xPrev - anchoMaximoArte / 2;
    for (int i = 0; i < lineasRepresentacion; ++i) {
        String^ linea = representacion[i]; if (linea == nullptr) continue;
        try {
            if (yActual >= 0 && yActual < Console::BufferHeight && xLimpiar >= 0 && xLimpiar + linea->Length <= Console::BufferWidth) {
                Console::SetCursorPosition(xLimpiar, yActual); Console::Write(gcnew String(L' ', linea->Length));
            } yActual++;
        }
        catch (ArgumentOutOfRangeException^) { break; }
    }
}

void Jugador::Actualizar(Mapa^ mapa, Jugador^ jugador) {}

bool Jugador::EnColision(ObjetoJuego^ otro) {
    Enemigo^ enemigo = dynamic_cast<Enemigo^>(otro); if (enemigo != nullptr) { RecibirDano(enemigo->ObtenerDano()); return false; }
    if (dynamic_cast<Aliado^>(otro) != nullptr) { return false; }
    if (dynamic_cast<Recurso^>(otro) != nullptr) { return false; }
    return true;
}

void Jugador::Mover(int dx, int dy) { EstablecerPosicion(x + dx, y + dy); }

void Jugador::RedimensionarInventario() {
    int nuevaCapacidad = (capacidadInventario == 0) ? CAPACIDAD_INICIAL_INVENTARIO : capacidadInventario * 2;
    array<Recurso^>^ nuevoInventario = gcnew array<Recurso^>(nuevaCapacidad);
    for (int i = 0; i < cantidadInventario; ++i) { nuevoInventario[i] = inventario[i]; }
    inventario = nuevoInventario; capacidadInventario = nuevaCapacidad;
}

void Jugador::RecolectarRecurso(Recurso^ recurso) {
    if (recurso != nullptr) {
        if (cantidadInventario == capacidadInventario) RedimensionarInventario();
        inventario[cantidadInventario++] = recurso;
        try {
            int msgY = Console::WindowHeight > 1 ? Console::WindowHeight - 2 : 0; if (msgY < 0) msgY = 0;
            int consoleWidth = Console::WindowWidth > 1 ? Console::WindowWidth - 2 : 0; if (consoleWidth < 0) consoleWidth = 0;
            String^ clearMsg = gcnew String(L' ', consoleWidth);
            Console::SetCursorPosition(1, msgY); Console::Write(clearMsg);
            Console::SetCursorPosition(1, msgY); Console::ForegroundColor = ConsoleColor::Yellow;
            Console::Write("Recurso: " + recurso->ObtenerNombre() + " recolectado."); Console::ResetColor();
        }
        catch (Exception^) {}
    }
}

void Jugador::RecibirDano(int cantidad) {
    if (cantidad <= 0) return;
    clock_t ahora = clock();
    double segundosDesdeUltimoDano = (double)(ahora - tiempoUltimoDano) / CLOCKS_PER_SEC;
    if (segundosDesdeUltimoDano >= COOLDOWN_DANO_SEGUNDOS) {
        vidas -= cantidad; if (vidas < 0) vidas = 0;
        tiempoUltimoDano = ahora;
        try {
            int msgY = Console::WindowHeight > 0 ? Console::WindowHeight - 1 : 0; if (msgY < 0) msgY = 0;
            int consoleWidth = Console::WindowWidth > 1 ? Console::WindowWidth - 2 : 0; if (consoleWidth < 0) consoleWidth = 0;
            String^ clearMsg = gcnew String(L' ', consoleWidth);
            Console::SetCursorPosition(1, msgY); Console::Write(clearMsg);
            Console::SetCursorPosition(1, msgY); Console::ForegroundColor = ConsoleColor::Red;
            Console::Write("¡Auch! Vidas restantes: " + vidas); Console::ResetColor();
        }
        catch (Exception^) {}
    }
}

int Jugador::ObtenerVidas() { return vidas; }
int Jugador::ObtenerEnergia() { return energia; }
int Jugador::ObtenerCantidadInventario() { return cantidadInventario; }
array<Recurso^>^ Jugador::ObtenerInventario() {
    array<Recurso^>^ inventarioActual = gcnew array<Recurso^>(cantidadInventario);
    for (int i = 0; i < cantidadInventario; ++i) inventarioActual[i] = inventario[i];
    return inventarioActual;
}