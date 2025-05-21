#include "ObjetoJuego.h"

void ObjetoJuego::EstablecerPosicion(int nuevoX, int nuevoY) {
    xPrev = x;
    yPrev = y;
    x = nuevoX;
    y = nuevoY;
}

void ObjetoJuego::AgregarLineaRepresentacion(String^ linea) {
    if (lineasRepresentacion < MAX_LINEAS_REPRESENTACION) {
        if (linea != nullptr) {
            representacion[lineasRepresentacion] = linea;
            lineasRepresentacion++;
        }
    }
}

int ObjetoJuego::ObtenerAnchoVisual() {
    int anchoMaximo = 0;
    for (int i = 0; i < lineasRepresentacion; ++i) {
        if (representacion[i] != nullptr && representacion[i]->Length > anchoMaximo) {
            anchoMaximo = representacion[i]->Length;
        }
    }
    return anchoMaximo;
}

int ObjetoJuego::ObtenerAltoVisual() {
    return lineasRepresentacion;
}

int ObjetoJuego::ObtenerXInicialDibujo() {
    return x - (ObtenerAnchoVisual() / 2);
}

int ObjetoJuego::ObtenerYInicialDibujo() {
    return y;
}

bool ObjetoJuego::HaySuperposicionVisual(ObjetoJuego^ otro) {
    if (otro == nullptr) return false;

    int thisAncho = this->ObtenerAnchoVisual();
    int thisAlto = this->ObtenerAltoVisual();
    if (thisAncho == 0 || thisAlto == 0) return false;

    int thisX1 = this->ObtenerXInicialDibujo();
    int thisY1 = this->ObtenerYInicialDibujo();
    int thisX2 = thisX1 + thisAncho - 1;
    int thisY2 = thisY1 + thisAlto - 1;

    int otroAncho = otro->ObtenerAnchoVisual();
    int otroAlto = otro->ObtenerAltoVisual();
    if (otroAncho == 0 || otroAlto == 0) return false;

    int otroX1 = otro->ObtenerXInicialDibujo();
    int otroY1 = otro->ObtenerYInicialDibujo();
    int otroX2 = otroX1 + otroAncho - 1;
    int otroY2 = otroY1 + otroAlto - 1;

    bool noSolapaX = thisX2 < otroX1 || thisX1 > otroX2;
    bool noSolapaY = thisY2 < otroY1 || thisY1 > otroY2;

    if (noSolapaX || noSolapaY) {
        return false;
    }
    return true;
}