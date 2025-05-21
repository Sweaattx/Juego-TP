#include "GestorJuego.h"
#include "Enemigo.h"
#include "Aliado.h"
#include "Recurso.h" 
#include "Jugador.h" 
#include "Mapa.h"    
#include "ObjetoJuego.h" 
#include <cstdlib>   
#include <ctime>     
#include <thread>    
#include <cmath>     
#include <cliext/map> 

using namespace System;
using namespace System::Threading;
using namespace System::Collections::Generic;
using namespace System::IO;

// Constructor
GestorJuego::GestorJuego() {
    this->mapaMundo1 = nullptr; this->mapaMundo2 = nullptr; this->mapaMundo3 = nullptr;
    this->mapaActual = nullptr; this->jugador = nullptr;
    this->estadoActual = GestorJuego::EstadoJuego::INICIALIZANDO;
    this->estadoActualAnterior = GestorJuego::EstadoJuego::INICIALIZANDO;
    this->estadoMundoSiguiente = GestorJuego::EstadoJuego::INICIALIZANDO;
    this->nombreMundoCargando = "N/A";
    this->progresoCargaActual = 0;
    this->tiempoInicioCarga = 0;
    this->juegoEnCurso = false; this->progresoMundo3 = 0;
    this->anchoConsola = 80; this->altoConsola = 24;
    this->opcionMenuPrincipalSel = 0; this->opcionDificultadSel = 1;
    this->dificultadJuego = GestorJuego::Dificultad::NORMAL;
    this->aliadosSpawneados = 0;
    this->necesitaRedibujarCompleto = true;
    this->tiempoInicio = 0;
    srand(static_cast<unsigned int>(time(0)));
    this->InicializarMenus();
    this->ultimoRecursoNombre = nullptr;
    this->mostrarMensajeRecurso = false;
    this->ultimoRecursoColor = ConsoleColor::Gray;
}

// Destructor
GestorJuego::~GestorJuego() {
    // GC se encarga
}

void GestorJuego::InicializarDatosJuego() {
    this->progresoMundo3 = 0;
    this->aliadosSpawneados = 0;
    this->mostrarMensajeRecurso = false;
    this->ultimoRecursoNombre = nullptr;
    this->progresoCargaActual = 0;
    this->tiempoInicioCarga = 0;
}

bool GestorJuego::GenerarPosicionValidaParaObjeto(Mapa^ mapa, ObjetoJuego^ objPrototipo, int% outX, int% outY, int intentosMax) {
    if (mapa == nullptr || objPrototipo == nullptr || mapa->ObtenerAncho() <= 2 || mapa->ObtenerAlto() <= 2) {
        if (mapa != nullptr) {
            outX = mapa->ObtenerAncho() / 2;
            outY = mapa->ObtenerAlto() / 2;
        }
        else {
            outX = 10; outY = 5;
        }
        return false;
    }
    int anchoObjVisualProto = objPrototipo->ObtenerAnchoVisual();
    int altoObjVisualProto = objPrototipo->ObtenerAltoVisual();

    int protoXOriginal = objPrototipo->ObtenerX();
    int protoYOriginal = objPrototipo->ObtenerY();

    for (int i = 0; i < intentosMax; ++i) {
        int tempXAnclaje = rand() % (mapa->ObtenerAncho() - 2) + 1;
        int tempYAnclaje = rand() % (mapa->ObtenerAlto() - 2) + 1;

        int xEsquinaProto = tempXAnclaje - (anchoObjVisualProto / 2);
        int yEsquinaProto = tempYAnclaje;

        if (mapa->EsAreaValidaParaObjeto(xEsquinaProto, yEsquinaProto, anchoObjVisualProto, altoObjVisualProto)) {
            bool areaVisualLibre = true;

            objPrototipo->EstablecerPosicion(tempXAnclaje, tempYAnclaje);

            array<ObjetoJuego^>^ todosLosObjetos = mapa->ObtenerTodosObjetos();
            for each(ObjetoJuego ^ existente in todosLosObjetos) {
                if (existente == nullptr || existente == objPrototipo || existente == jugador) continue;

                if (objPrototipo->HaySuperposicionVisual(existente)) {
                    areaVisualLibre = false;
                    break;
                }
            }

            objPrototipo->EstablecerPosicion(protoXOriginal, protoYOriginal);

            if (areaVisualLibre) {
                outX = tempXAnclaje; outY = tempYAnclaje;
                return true;
            }
        }
    }

    objPrototipo->EstablecerPosicion(protoXOriginal, protoYOriginal);

    outX = mapa->ObtenerAncho() / 2; outY = mapa->ObtenerAlto() / 2;
    int xEsqFallback = outX - (anchoObjVisualProto / 2); int yEsqFallback = outY;
    if (mapa->EsAreaValidaParaObjeto(xEsqFallback, yEsqFallback, anchoObjVisualProto, altoObjVisualProto)) return true;

    outX = 1 + (anchoObjVisualProto / 2);
    outY = 1;

    xEsqFallback = outX - (anchoObjVisualProto / 2); yEsqFallback = outY;
    if (mapa->EsAreaValidaParaObjeto(xEsqFallback, yEsqFallback, anchoObjVisualProto, altoObjVisualProto)) return true;

    return false;
}

bool GestorJuego::GenerarPosicionValidaParaObjeto(Mapa^ mapa, ObjetoJuego^ objPrototipo, int% outX, int% outY) {
    return this->GenerarPosicionValidaParaObjeto(mapa, objPrototipo, outX, outY, 100);
}

void GestorJuego::Ejecutar() {
    ConsoleColor fgOriginal = Console::ForegroundColor;
    ConsoleColor bgOriginal = Console::BackgroundColor;
    bool cursorVisibleOriginal = Console::CursorVisible;

    Console::BackgroundColor = ConsoleColor::Black;
    try { Console::Clear(); }
    catch (Exception^) {}

    try {
        this->InicializarConsolaYObtenerTamano();
        this->estadoActual = GestorJuego::EstadoJuego::MENU_PRINCIPAL;
        this->juegoEnCurso = true;
        this->necesitaRedibujarCompleto = true;

        this->BucleJuego();
    }
    catch (System::Exception^ ex) {
        try {
            Console::ForegroundColor = ConsoleColor::Red; Console::BackgroundColor = ConsoleColor::Black;
            Console::Clear();
            Console::WriteLine("ERROR CRITICO DEL SISTEMA EN Ejecutar(): " + ex->Message);
            Console::WriteLine(ex->StackTrace);
            Console::ResetColor(); Console::WriteLine("\nPresiona una tecla para salir."); Console::ReadKey(true);
        }
        catch (Exception^ eInterno) { /* Usar eInterno o quitarlo */ Console::Error->WriteLine("Error en catch anidado: " + eInterno->Message); }
    }
    catch (...) {
        try {
            Console::ForegroundColor = ConsoleColor::Red; Console::BackgroundColor = ConsoleColor::Black;
            Console::Clear();
            Console::WriteLine("ERROR CRITICO DESCONOCIDO EN Ejecutar().");
            Console::ResetColor(); Console::WriteLine("\nPresiona una tecla para salir."); Console::ReadKey(true);
        }
        catch (Exception^) {}
    }
    finally {
        try {
            Console::ForegroundColor = fgOriginal; Console::BackgroundColor = bgOriginal;
            Console::CursorVisible = cursorVisibleOriginal;
            Console::Clear();
        }
        catch (Exception^) {}
    }
}

void GestorJuego::InicializarConsolaYObtenerTamano() {
    Console::Title = "Vigilante del Equilibrio TP - Ayvar, Chavez, Ramos";
    Console::CursorVisible = false;
    Console::BackgroundColor = ConsoleColor::Black;

    int anchoDeseado = 160; int altoDeseadoConsola = 40;
    try {
        int maxW = Console::LargestWindowWidth; int maxH = Console::LargestWindowHeight;
        int targetW = Math::Min(anchoDeseado, maxW);
        int targetH = Math::Min(altoDeseadoConsola, maxH);

        int bufferW = Math::Max(targetW, Console::WindowWidth);
        int bufferH = Math::Max(targetH + 2, Console::WindowHeight);

        if (Console::BufferWidth < bufferW || Console::BufferHeight < bufferH) {
            int currentWindowW = Console::WindowWidth;
            int currentWindowH = Console::WindowHeight;
            if (bufferW < currentWindowW && !Console::IsOutputRedirected) Console::SetWindowSize(Math::Max(1, bufferW), currentWindowH);
            if (bufferH < currentWindowH && !Console::IsOutputRedirected) Console::SetWindowSize(currentWindowW, Math::Max(1, bufferH));
            Console::SetBufferSize(bufferW, bufferH);
        }
        if (targetW > 0 && targetH > 0 && targetW <= Console::BufferWidth && targetH <= Console::BufferHeight && !Console::IsOutputRedirected) {
            Console::SetWindowSize(targetW, targetH);
        }
    }
    catch (IO::IOException^ ioEx) {
        Console::Error->WriteLine("Advertencia: IOException al inicializar consola: " + ioEx->Message);
        try {
            if (Console::BufferWidth < 80 || Console::BufferHeight < 25) Console::SetBufferSize(80, 300);
            if (!Console::IsOutputRedirected) Console::SetWindowSize(80, 25);
        }
        catch (Exception^) {}
    }
    catch (ArgumentOutOfRangeException^ argEx) {
        Console::Error->WriteLine("Advertencia: ArgumentOutOfRangeException al inicializar consola: " + argEx->Message);
        try {
            if (Console::BufferWidth < 80 || Console::BufferHeight < 25) Console::SetBufferSize(80, 300);
            if (!Console::IsOutputRedirected) Console::SetWindowSize(80, 25);
        }
        catch (Exception^) {}
    }
    catch (Exception^ ex) {
        Console::Error->WriteLine("Advertencia: Error general al inicializar consola: " + ex->Message);
        try {
            if (Console::BufferWidth < 80 || Console::BufferHeight < 25) Console::SetBufferSize(80, 300);
            if (!Console::IsOutputRedirected) Console::SetWindowSize(80, 25);
        }
        catch (Exception^) {}
    }

    try {
        this->anchoConsola = Console::WindowWidth;
        this->altoConsola = Console::WindowHeight > 1 ? Console::WindowHeight - 1 : 24;
    }
    catch (Exception^) { this->anchoConsola = 80; this->altoConsola = 24; }

    if (this->anchoConsola < 60) this->anchoConsola = 60;
    if (this->altoConsola < 20) this->altoConsola = 20;
}

void GestorJuego::InicializarMundosYJugador() {
    this->mapaMundo1 = nullptr; this->mapaMundo2 = nullptr; this->mapaMundo3 = nullptr;
    this->jugador = nullptr; this->mapaActual = nullptr;

    int alturaEfectivaMapas = this->altoConsola - 2;
    if (alturaEfectivaMapas < 10) alturaEfectivaMapas = 10;

    int posYJugador = alturaEfectivaMapas - 3;
    if (posYJugador < 1) posYJugador = 1;

    Jugador^ tempJugadorParaAlto = gcnew Jugador(0, 0, 1, 1);
    if (posYJugador < tempJugadorParaAlto->ObtenerAltoVisual()) posYJugador = tempJugadorParaAlto->ObtenerAltoVisual();
    if (posYJugador >= alturaEfectivaMapas - tempJugadorParaAlto->ObtenerAltoVisual()) {
        posYJugador = alturaEfectivaMapas - tempJugadorParaAlto->ObtenerAltoVisual() - 1;
        if (posYJugador < 1) posYJugador = 1;
    }
    tempJugadorParaAlto = nullptr;

    this->jugador = gcnew Jugador(this->anchoConsola / 2, posYJugador, JUGADOR_VIDAS_INICIO, JUGADOR_ENERGIA_INICIO);

    this->mapaMundo1 = gcnew Mapa(this->anchoConsola, alturaEfectivaMapas, "Mundo 1: Humanitas", ConsoleColor::Black);
    this->mapaMundo2 = gcnew Mapa(this->anchoConsola, alturaEfectivaMapas, "Mundo 2: Technos", ConsoleColor::Black);
    this->mapaMundo3 = gcnew Mapa(this->anchoConsola, alturaEfectivaMapas, "Mundo 3: Equilibrium", ConsoleColor::Black);

    const int TOTAL_CATEGORIAS_RECURSOS_POR_MUNDO = 4;
    const int MIN_ENEMIGOS_POR_MUNDO = 3;

    int numRecursosPorTipoM1 = 2, numEnemigosM1 = 4, velEnemigoM1Min = 4, velEnemigoM1Max = 6, danoEnemigoM1 = 1;
    int numRecursosPorTipoM2 = 2, numEnemigosM2 = 4, velEnemigoM2Min = 2, velEnemigoM2Max = 4, danoEnemigoM2 = 1;

    if (this->dificultadJuego == GestorJuego::Dificultad::FACIL) {
        numRecursosPorTipoM1 = 3; numEnemigosM1 = 3; velEnemigoM1Min = 5; velEnemigoM1Max = 7;
        numRecursosPorTipoM2 = 3; numEnemigosM2 = 3; velEnemigoM2Min = 3; velEnemigoM2Max = 5; danoEnemigoM2 = 1;
    }
    else if (this->dificultadJuego == GestorJuego::Dificultad::DIFICIL) {
        numRecursosPorTipoM1 = 1; numEnemigosM1 = 5; velEnemigoM1Min = 3; velEnemigoM1Max = 5;
        numRecursosPorTipoM2 = 1; numEnemigosM2 = 6; velEnemigoM2Min = 1; velEnemigoM2Max = 3; danoEnemigoM2 = 2;
    }

    while (numRecursosPorTipoM1 * TOTAL_CATEGORIAS_RECURSOS_POR_MUNDO < 6 && numRecursosPorTipoM1 < 5) numRecursosPorTipoM1++;
    while (numRecursosPorTipoM2 * TOTAL_CATEGORIAS_RECURSOS_POR_MUNDO < 6 && numRecursosPorTipoM2 < 5) numRecursosPorTipoM2++;
    numEnemigosM1 = Math::Max(MIN_ENEMIGOS_POR_MUNDO, numEnemigosM1);
    numEnemigosM2 = Math::Max(MIN_ENEMIGOS_POR_MUNDO, numEnemigosM2);

    array<Recurso^>^ plantillasRecursosM1 = gcnew array<Recurso^>(4) {
        gcnew Recurso(0, 0, ConsoleColor::Green, TipoRecurso::HABILIDAD_HUMANA_EMPATIA, "Semilla Ancestral", 5, "  ^  ", " / \\ "),
            gcnew Recurso(0, 0, ConsoleColor::DarkYellow, TipoRecurso::HABILIDAD_HUMANA_ETICA, "Madera Noble", 5, ".==.", "|  |"),
            gcnew Recurso(0, 0, ConsoleColor::Cyan, TipoRecurso::HABILIDAD_HUMANA_ADAPTABILIDAD, "Agua Pura", 7, "  o  ", " / \\ "),
            gcnew Recurso(0, 0, ConsoleColor::Gray, TipoRecurso::HABILIDAD_HUMANA_CREATIVIDAD, "Cristal Natural", 6, " /\\ ", "<  >")
    };

    for each(Recurso ^ plantilla in plantillasRecursosM1) {
        for (int i = 0; i < numRecursosPorTipoM1; ++i) {
            int recX, recY;
            if (this->GenerarPosicionValidaParaObjeto(this->mapaMundo1, plantilla, recX, recY)) {
                String^ r1 = (plantilla->ObtenerLineasRepresentacion() > 0) ? plantilla->ObtenerRepresentacion()[0] : nullptr;
                String^ r2 = (plantilla->ObtenerLineasRepresentacion() > 1) ? plantilla->ObtenerRepresentacion()[1] : nullptr;
                this->mapaMundo1->AgregarObjeto(gcnew Recurso(recX, recY, plantilla->ObtenerColor(), plantilla->ObtenerTipoRecurso(), plantilla->ObtenerNombre(), plantilla->ObtenerValor(), r1, r2));
            }
        }
    }
    Enemigo^ enemigoPrototipoM1 = gcnew Enemigo(0, 0, ConsoleColor::Red, danoEnemigoM1, 1, L'X');
    for (int i = 0; i < numEnemigosM1; ++i) {
        int velocidad = rand() % (velEnemigoM1Max - velEnemigoM1Min + 1) + velEnemigoM1Min;
        wchar_t tipoEnemigoM1 = (rand() % 2 == 0) ? L'X' : L'O';
        enemigoPrototipoM1 = gcnew Enemigo(0, 0, ConsoleColor::Red, danoEnemigoM1, velocidad, tipoEnemigoM1);
        int enX, enY;
        if (this->GenerarPosicionValidaParaObjeto(this->mapaMundo1, enemigoPrototipoM1, enX, enY)) {
            this->mapaMundo1->AgregarObjeto(gcnew Enemigo(enX, enY, ConsoleColor::Red, danoEnemigoM1, velocidad, tipoEnemigoM1));
        }
    }
    enemigoPrototipoM1 = nullptr;


    array<Recurso^>^ plantillasRecursosM2 = gcnew array<Recurso^>(4) {
        gcnew Recurso(0, 0, ConsoleColor::White, TipoRecurso::TECNOLOGIA_IA, "Chip IA Avanzado", 10, ".---.", "|[o]|"),
            gcnew Recurso(0, 0, ConsoleColor::DarkCyan, TipoRecurso::TECNOLOGIA_BIG_DATA, "Nucleo de Datos", 9, "::::", "[DD]"),
            gcnew Recurso(0, 0, ConsoleColor::Yellow, TipoRecurso::TECNOLOGIA_ENERGIA, "Celda Energetica", 10, " +++ ", "( * )"),
            gcnew Recurso(0, 0, ConsoleColor::Blue, TipoRecurso::TECNOLOGIA_AUTOMATIZACION, "NanoEnsamblador", 9, "o-o-o", "| | |")
    };
    for each(Recurso ^ plantilla in plantillasRecursosM2) {
        for (int i = 0; i < numRecursosPorTipoM2; ++i) {
            int recX, recY;
            if (this->GenerarPosicionValidaParaObjeto(this->mapaMundo2, plantilla, recX, recY)) {
                String^ r1 = (plantilla->ObtenerLineasRepresentacion() > 0) ? plantilla->ObtenerRepresentacion()[0] : nullptr;
                String^ r2 = (plantilla->ObtenerLineasRepresentacion() > 1) ? plantilla->ObtenerRepresentacion()[1] : nullptr;
                this->mapaMundo2->AgregarObjeto(gcnew Recurso(recX, recY, plantilla->ObtenerColor(), plantilla->ObtenerTipoRecurso(), plantilla->ObtenerNombre(), plantilla->ObtenerValor(), r1, r2));
            }
        }
    }
    Enemigo^ enemigoPrototipoM2 = gcnew Enemigo(0, 0, ConsoleColor::Red, danoEnemigoM2, 1, L'#');
    for (int i = 0; i < numEnemigosM2; ++i) {
        int velocidad = rand() % (velEnemigoM2Max - velEnemigoM2Min + 1) + velEnemigoM2Min;
        wchar_t tipoEnemigoM2 = (rand() % 2 == 0) ? L'#' : L'*';
        enemigoPrototipoM2 = gcnew Enemigo(0, 0, ConsoleColor::Red, danoEnemigoM2, velocidad, tipoEnemigoM2);
        int enX, enY;
        if (this->GenerarPosicionValidaParaObjeto(this->mapaMundo2, enemigoPrototipoM2, enX, enY)) {
            this->mapaMundo2->AgregarObjeto(gcnew Enemigo(enX, enY, ConsoleColor::Red, danoEnemigoM2, velocidad, tipoEnemigoM2));
        }
    }
    enemigoPrototipoM2 = nullptr;

    this->mapaActual = this->mapaMundo1;
    int jX = 0, jY = 0;
    if (this->jugador != nullptr && this->GenerarPosicionValidaParaObjeto(this->mapaActual, this->jugador, jX, jY, 50)) {
        this->jugador->EstablecerPosicion(jX, jY);
    }
    else {
        int nuevaYJugador = this->mapaActual->ObtenerAlto() - (this->jugador != nullptr ? this->jugador->ObtenerAltoVisual() + 1 : 2);
        if (nuevaYJugador < 1) nuevaYJugador = 1;
        if (this->jugador != nullptr) this->jugador->EstablecerPosicion(this->mapaActual->ObtenerAncho() / 2, nuevaYJugador);
    }
    if (this->jugador != nullptr) this->mapaActual->AgregarObjeto(this->jugador);
}


void GestorJuego::InicializarMenus() {
    this->elementosMenuPrincipal = gcnew array<String^>(OPCIONES_MENU_PRINCIPAL) {
        "1. Iniciar Juego", "2. Instrucciones", "3. Integrantes", "4. Creditos", "5. Salir"
    };
    this->elementosMenuDificultad = gcnew array<String^>(OPCIONES_MENU_DIFICULTAD) {
        "  Facil  ", " Normal  ", " Dificil "
    };
    this->opcionDificultadSel = 1;
    if (this->opcionDificultadSel >= 0 && this->opcionDificultadSel < this->elementosMenuDificultad->Length) {
        this->elementosMenuDificultad[this->opcionDificultadSel] = ">" + this->elementosMenuDificultad[this->opcionDificultadSel]->Trim() + "<";
    }
}

// DEFINICIÓN COMPLETA DE RenderizarJuego
void GestorJuego::RenderizarJuego() {
    if (this->estadoActual != GestorJuego::EstadoJuego::MUNDO1 &&
        this->estadoActual != GestorJuego::EstadoJuego::MUNDO2 &&
        this->estadoActual != GestorJuego::EstadoJuego::VISTA_MUNDO3 &&
        this->estadoActual != GestorJuego::EstadoJuego::PANTALLA_CARGA) {
        try { Console::SetCursorPosition(0, 0); }
        catch (Exception^) {}
    }

    switch (this->estadoActual) {
    case GestorJuego::EstadoJuego::MENU_PRINCIPAL:       this->RenderizarMenuPrincipal(); break;
    case GestorJuego::EstadoJuego::MENU_DIFICULTAD:      this->RenderizarMenuDificultad(); break;
    case GestorJuego::EstadoJuego::MOSTRAR_INSTRUCCIONES:this->RenderizarPantallaInstrucciones(); break;
    case GestorJuego::EstadoJuego::MOSTRAR_INTEGRANTES:  this->RenderizarPantallaIntegrantes(); break;
    case GestorJuego::EstadoJuego::MOSTRAR_CREDITOS:     this->RenderizarPantallaCreditos(); break;
    case GestorJuego::EstadoJuego::MUNDO1:
    case GestorJuego::EstadoJuego::MUNDO2:               this->RenderizarMundo(); break;
    case GestorJuego::EstadoJuego::VISTA_MUNDO3:         this->RenderizarVistaMundo3(); break;
    case GestorJuego::EstadoJuego::PANTALLA_CARGA:       this->RenderizarPantallaCarga(); break;
    case GestorJuego::EstadoJuego::FIN_JUEGO_VICTORIA:
    case GestorJuego::EstadoJuego::FIN_JUEGO_DERROTA:    this->RenderizarFinJuego(); break;
    case GestorJuego::EstadoJuego::INICIALIZANDO:
        try {
            Console::SetCursorPosition(Math::Max(0, this->anchoConsola / 2 - 5), Math::Max(0, this->altoConsola / 2));
            Console::Write("Cargando...");
        }
        catch (Exception^) {} break;
    default:
        break;
    }

    if (this->estadoActual == GestorJuego::EstadoJuego::MUNDO1 || this->estadoActual == GestorJuego::EstadoJuego::MUNDO2 || this->estadoActual == GestorJuego::EstadoJuego::VISTA_MUNDO3) {
        this->MostrarHUD();
    }
    if (this->estadoActual == GestorJuego::EstadoJuego::MUNDO1 || this->estadoActual == GestorJuego::EstadoJuego::MUNDO2) {
        this->MostrarInventarioYRecoleccionHUD();
    }
    else if (this->estadoActual == GestorJuego::EstadoJuego::VISTA_MUNDO3) {
        int posYInventario = this->altoConsola;
        if (posYInventario >= 1 && posYInventario < Console::BufferHeight) {
            this->LimpiarAreaConsola(0, posYInventario, this->anchoConsola, 1);
        }
    }

    if (this->estadoActual != GestorJuego::EstadoJuego::MUNDO1 &&
        this->estadoActual != GestorJuego::EstadoJuego::MUNDO2 &&
        this->estadoActual != GestorJuego::EstadoJuego::VISTA_MUNDO3 &&
        this->estadoActual != GestorJuego::EstadoJuego::PANTALLA_CARGA) {
        try {
            int finalCursorY = Console::WindowHeight - 1;
            if (finalCursorY < 0) finalCursorY = 0;
            if (finalCursorY < Console::BufferHeight) {
                Console::SetCursorPosition(0, finalCursorY);
            }
        }
        catch (Exception^) {
            try { Console::SetCursorPosition(0, 0); }
            catch (Exception^) {}
        }
    }
}


void GestorJuego::BucleJuego() {
    while (this->juegoEnCurso) {
        if (this->necesitaRedibujarCompleto) {
            Console::BackgroundColor = ConsoleColor::Black;
            try { Console::Clear(); }
            catch (Exception^) {}
            // RenderizarJuego se llamará después de este bloque si necesitaRedibujarCompleto era true,
            // o si es un estado de juego activo.
        }

        this->ProcesarEntrada();
        if (!this->juegoEnCurso) break;

        EstadoJuego estadoAntesDelUpdate = this->estadoActual;
        bool esJuegoActivoAntes = (estadoAntesDelUpdate == GestorJuego::EstadoJuego::MUNDO1 || estadoAntesDelUpdate == GestorJuego::EstadoJuego::MUNDO2);

        if (this->estadoActual == GestorJuego::EstadoJuego::PANTALLA_CARGA) {
            // RenderizarPantallaCarga ya hace su propio Clear()
            clock_t tiempoActualCarga = clock();
            double elapsed_secs = (this->tiempoInicioCarga == 0) ? DURACION_PANTALLA_CARGA_SEGUNDOS + 1 : double(tiempoActualCarga - this->tiempoInicioCarga) / CLOCKS_PER_SEC;
            this->progresoCargaActual = static_cast<int>((elapsed_secs / DURACION_PANTALLA_CARGA_SEGUNDOS) * 100.0);

            if (this->progresoCargaActual >= 100) {
                this->progresoCargaActual = 100;
                this->RenderizarPantallaCarga();
                Thread::Sleep(150);

                Mapa^ mapaDestinoReal = nullptr;
                switch (this->estadoMundoSiguiente) {
                case GestorJuego::EstadoJuego::MUNDO1: mapaDestinoReal = this->mapaMundo1; break;
                case GestorJuego::EstadoJuego::MUNDO2: mapaDestinoReal = this->mapaMundo2; break;
                case GestorJuego::EstadoJuego::VISTA_MUNDO3: mapaDestinoReal = this->mapaMundo3; break;
                default:
                    this->estadoActual = GestorJuego::EstadoJuego::MENU_PRINCIPAL;
                    this->necesitaRedibujarCompleto = true;
                    continue;
                }

                if (mapaDestinoReal == nullptr) {
                    this->estadoActual = GestorJuego::EstadoJuego::MENU_PRINCIPAL;
                }
                else {
                    if (this->mapaActual != nullptr && this->jugador != nullptr &&
                        (this->estadoActualAnterior == GestorJuego::EstadoJuego::MUNDO1 || this->estadoActualAnterior == GestorJuego::EstadoJuego::MUNDO2) &&
                        this->mapaActual != mapaDestinoReal) {
                        this->mapaActual->EliminarObjeto(this->jugador);
                    }
                    this->estadoActual = this->estadoMundoSiguiente;
                    this->mapaActual = mapaDestinoReal;
                    if (this->estadoActual == GestorJuego::EstadoJuego::MUNDO1 || this->estadoActual == GestorJuego::EstadoJuego::MUNDO2) {
                        bool jugadorYaExiste = false;
                        if (this->mapaActual != nullptr && this->jugador != nullptr) {
                            array<ObjetoJuego^>^ objs = this->mapaActual->ObtenerTodosObjetos();
                            for (int i = 0; i < objs->Length; ++i) { if (objs[i] == this->jugador) { jugadorYaExiste = true; break; } }
                            if (!jugadorYaExiste) this->mapaActual->AgregarObjeto(this->jugador);
                        }
                        int jX = 0, jY = 0;
                        if (this->jugador != nullptr && !this->GenerarPosicionValidaParaObjeto(this->mapaActual, this->jugador, jX, jY, 50)) {
                            jX = this->mapaActual->ObtenerAncho() / 2;
                            jY = this->mapaActual->ObtenerAlto() - (this->jugador->ObtenerAltoVisual() > 0 ? this->jugador->ObtenerAltoVisual() + 1 : 2);
                            if (jY < 1) jY = 1;
                        }
                        if (this->jugador != nullptr) this->jugador->EstablecerPosicion(jX, jY);
                    }
                }
                this->necesitaRedibujarCompleto = true;
            }
            else {
                this->RenderizarPantallaCarga();
            }
        }
        else if (esJuegoActivoAntes && this->estadoActual == estadoAntesDelUpdate) {
            this->ActualizarJuego();
            this->VerificarCondicionesFin();
            if (!this->juegoEnCurso) {
                this->necesitaRedibujarCompleto = true;
            }
        }

        if (this->necesitaRedibujarCompleto) {
            Console::BackgroundColor = ConsoleColor::Black;
            try { Console::Clear(); }
            catch (Exception^) {}
            this->RenderizarJuego();
            this->necesitaRedibujarCompleto = false;
        }
        else if (this->estadoActual == GestorJuego::EstadoJuego::MUNDO1 ||
            this->estadoActual == GestorJuego::EstadoJuego::MUNDO2 ||
            this->estadoActual == GestorJuego::EstadoJuego::VISTA_MUNDO3) {
            this->RenderizarJuego();
        }

        if (!juegoEnCurso) {
            break;
        }

        Thread::Sleep(50);
    }

    if (this->estadoActual == GestorJuego::EstadoJuego::FIN_JUEGO_VICTORIA || this->estadoActual == GestorJuego::EstadoJuego::FIN_JUEGO_DERROTA) {
        // RenderizarJuego ya fue llamado si necesitaRedibujarCompleto era true.
        // Si no, forzar una última renderización de la pantalla final.
        if (!this->necesitaRedibujarCompleto) { // Si no se limpió y redibujó arriba
            Console::BackgroundColor = ConsoleColor::Black; try { Console::Clear(); }
            catch (Exception^) {}
            this->RenderizarJuego();
        }
    }
    else { // Si salió por ESC u otra razón
        Console::BackgroundColor = ConsoleColor::Black; try { Console::Clear(); }
        catch (Exception^) {}
    }


    while (Console::KeyAvailable) {
        Console::ReadKey(true);
    }
    String^ msgFinal = "Presiona CUALQUIER tecla para salir del juego...";
    try {
        Console::ResetColor();
        Console::BackgroundColor = ConsoleColor::Black;
        Console::ForegroundColor = ConsoleColor::White;

        int lineaMsgFinal = Console::BufferHeight > 0 ? Console::BufferHeight - 1 : 0;
        if (lineaMsgFinal >= 0 && lineaMsgFinal < Console::BufferHeight) {
            this->LimpiarAreaConsola(0, lineaMsgFinal, this->anchoConsola, 1);
            int msgX = Math::Max(0, (Console::WindowWidth - msgFinal->Length) / 2);
            if (msgX + msgFinal->Length >= Console::WindowWidth && Console::WindowWidth > 0) msgX = Math::Max(0, Console::WindowWidth - msgFinal->Length - 1);
            if (msgX < 0) msgX = 0;
            if (msgX + msgFinal->Length <= Console::BufferWidth) {
                Console::SetCursorPosition(msgX, lineaMsgFinal);
                Console::Write(msgFinal);
            }
            else {
                Console::WriteLine("\n" + msgFinal);
            }
        }
        else {
            Console::WriteLine("\n\n" + msgFinal);
        }
    }
    catch (Exception^ ex) {
        try { Console::Clear(); Console::WriteLine("\n\nJuego terminado."); Console::WriteLine(msgFinal + " (Error al mostrar msg final: " + ex->Message + ")"); }
        catch (Exception^) {}
    }
    Console::ReadKey(true);
}

void GestorJuego::ProcesarEntrada() {
    if (!Console::KeyAvailable) return;
    ConsoleKeyInfo infoTecla = Console::ReadKey(true);

    EstadoJuego estadoAntesDeProcesar = this->estadoActual;

    if (infoTecla.Key == ConsoleKey::Escape) {
        if (this->estadoActual == GestorJuego::EstadoJuego::MOSTRAR_INSTRUCCIONES ||
            this->estadoActual == GestorJuego::EstadoJuego::MOSTRAR_INTEGRANTES ||
            this->estadoActual == GestorJuego::EstadoJuego::MOSTRAR_CREDITOS ||
            this->estadoActual == GestorJuego::EstadoJuego::MENU_DIFICULTAD ||
            this->estadoActual == GestorJuego::EstadoJuego::PANTALLA_CARGA) {
            this->estadoActual = GestorJuego::EstadoJuego::MENU_PRINCIPAL;
        }
        else if (this->estadoActual == GestorJuego::EstadoJuego::MUNDO1 ||
            this->estadoActual == GestorJuego::EstadoJuego::MUNDO2 ||
            this->estadoActual == GestorJuego::EstadoJuego::VISTA_MUNDO3) {
            this->estadoActual = GestorJuego::EstadoJuego::MENU_PRINCIPAL;
        }
        else if (this->estadoActual == GestorJuego::EstadoJuego::MENU_PRINCIPAL) {
            this->juegoEnCurso = false;
        }
        else if (this->estadoActual == GestorJuego::EstadoJuego::FIN_JUEGO_VICTORIA ||
            this->estadoActual == GestorJuego::EstadoJuego::FIN_JUEGO_DERROTA) {
            this->juegoEnCurso = false;
        }

        if (estadoAntesDeProcesar != this->estadoActual || !this->juegoEnCurso) {
            this->necesitaRedibujarCompleto = true;
        }
        return;
    }

    if (this->estadoActual != GestorJuego::EstadoJuego::PANTALLA_CARGA) {
        switch (this->estadoActual) {
        case GestorJuego::EstadoJuego::MENU_PRINCIPAL:       this->ProcesarEntradaMenuPrincipal(infoTecla); break;
        case GestorJuego::EstadoJuego::MENU_DIFICULTAD:      this->ProcesarEntradaMenuDificultad(infoTecla); break;
        case GestorJuego::EstadoJuego::MOSTRAR_INSTRUCCIONES:
        case GestorJuego::EstadoJuego::MOSTRAR_INTEGRANTES:
        case GestorJuego::EstadoJuego::MOSTRAR_CREDITOS:     this->ProcesarEntradaPantallaInfo(infoTecla); break;
        case GestorJuego::EstadoJuego::MUNDO1:
        case GestorJuego::EstadoJuego::MUNDO2: // CORREGIDO
        case GestorJuego::EstadoJuego::VISTA_MUNDO3:         this->ProcesarEntradaMundo(infoTecla); break;
        case GestorJuego::EstadoJuego::FIN_JUEGO_VICTORIA:
        case GestorJuego::EstadoJuego::FIN_JUEGO_DERROTA:    this->ProcesarEntradaFinJuego(infoTecla); break;
        default: break;
        }
    }
    if (estadoAntesDeProcesar != this->estadoActual && this->estadoActual != GestorJuego::EstadoJuego::PANTALLA_CARGA) {
        this->necesitaRedibujarCompleto = true;
    }
}

void GestorJuego::ProcesarEntradaMenuPrincipal(ConsoleKeyInfo infoTecla) {
    int opcionPrevia = this->opcionMenuPrincipalSel;
    switch (infoTecla.Key) {
    case ConsoleKey::UpArrow: case ConsoleKey::W:
        this->opcionMenuPrincipalSel = (this->opcionMenuPrincipalSel - 1 + this->elementosMenuPrincipal->Length) % this->elementosMenuPrincipal->Length;
        break;
    case ConsoleKey::DownArrow: case ConsoleKey::S:
        this->opcionMenuPrincipalSel = (this->opcionMenuPrincipalSel + 1) % this->elementosMenuPrincipal->Length;
        break;
    case ConsoleKey::Enter:
        switch (this->opcionMenuPrincipalSel) {
        case 0: this->estadoActual = GestorJuego::EstadoJuego::MENU_DIFICULTAD; this->opcionDificultadSel = 1; break;
        case 1: this->estadoActual = GestorJuego::EstadoJuego::MOSTRAR_INSTRUCCIONES; break;
        case 2: this->estadoActual = GestorJuego::EstadoJuego::MOSTRAR_INTEGRANTES; break;
        case 3: this->estadoActual = GestorJuego::EstadoJuego::MOSTRAR_CREDITOS; break;
        case 4: this->juegoEnCurso = false; break;
        }
        // necesitaRedibujarCompleto se actualiza en ProcesarEntrada si el estado cambió
        break;
    }
    if (opcionPrevia != this->opcionMenuPrincipalSel) this->necesitaRedibujarCompleto = true;
}

void GestorJuego::ProcesarEntradaMenuDificultad(ConsoleKeyInfo infoTecla) {
    int opcionPrevia = this->opcionDificultadSel;
    switch (infoTecla.Key) {
    case ConsoleKey::LeftArrow: case ConsoleKey::A:
        this->opcionDificultadSel = (this->opcionDificultadSel - 1 + this->elementosMenuDificultad->Length) % this->elementosMenuDificultad->Length;
        break;
    case ConsoleKey::RightArrow: case ConsoleKey::D:
        this->opcionDificultadSel = (this->opcionDificultadSel + 1) % this->elementosMenuDificultad->Length;
        break;
    case ConsoleKey::Enter:
        if (this->opcionDificultadSel == 0) this->dificultadJuego = GestorJuego::Dificultad::FACIL;
        else if (this->opcionDificultadSel == 1) this->dificultadJuego = GestorJuego::Dificultad::NORMAL;
        else this->dificultadJuego = GestorJuego::Dificultad::DIFICIL;
        this->IniciarJuegoNuevo(this->dificultadJuego);
        return;
    }

    if (opcionPrevia != this->opcionDificultadSel) {
        this->necesitaRedibujarCompleto = true;
        this->elementosMenuDificultad[0] = "  Facil  ";
        this->elementosMenuDificultad[1] = " Normal  ";
        this->elementosMenuDificultad[2] = " Dificil ";
        this->elementosMenuDificultad[this->opcionDificultadSel] = ">" + this->elementosMenuDificultad[this->opcionDificultadSel]->Trim() + "<";
    }
}

void GestorJuego::ProcesarEntradaPantallaInfo(ConsoleKeyInfo infoTecla) {
    // ESC es manejado globalmente. 
}

void GestorJuego::ProcesarEntradaMundo(ConsoleKeyInfo infoTecla) {
    int dx = 0, dy = 0;
    EstadoJuego estadoOriginal = this->estadoActual;

    if (infoTecla.Key == ConsoleKey::D1 && this->estadoActual != GestorJuego::EstadoJuego::MUNDO1) { this->CambiarMundo(GestorJuego::EstadoJuego::MUNDO1); }
    else if (infoTecla.Key == ConsoleKey::D2 && this->estadoActual != GestorJuego::EstadoJuego::MUNDO2) { this->CambiarMundo(GestorJuego::EstadoJuego::MUNDO2); }
    else if (infoTecla.Key == ConsoleKey::D3 && this->estadoActual != GestorJuego::EstadoJuego::VISTA_MUNDO3) { this->CambiarMundo(GestorJuego::EstadoJuego::VISTA_MUNDO3); }
    else if (estadoOriginal == GestorJuego::EstadoJuego::MUNDO1 || estadoOriginal == GestorJuego::EstadoJuego::MUNDO2) {
        switch (infoTecla.Key) {
        case ConsoleKey::UpArrow: case ConsoleKey::W: dy = -1; break;
        case ConsoleKey::DownArrow: case ConsoleKey::S: dy = 1; break;
        case ConsoleKey::LeftArrow: case ConsoleKey::A: dx = -1; break;
        case ConsoleKey::RightArrow: case ConsoleKey::D: dx = 1; break;
        case ConsoleKey::H: this->IntentarInvocarAliado(); break;
        default: break;
        }
        if (dx != 0 || dy != 0) {
            this->ManejarMovimientoJugador(dx, dy);
        }
    }
}

void GestorJuego::ProcesarEntradaFinJuego(ConsoleKeyInfo infoTecla) {
    this->juegoEnCurso = false;
}

void GestorJuego::ManejarMovimientoJugador(int dx, int dy) {
    if (this->jugador == nullptr || this->mapaActual == nullptr || (dx == 0 && dy == 0) ||
        (this->estadoActual != GestorJuego::EstadoJuego::MUNDO1 && this->estadoActual != GestorJuego::EstadoJuego::MUNDO2)) return;

    int xSiguienteAnclaje = this->jugador->ObtenerX() + dx;
    int ySiguienteAnclaje = this->jugador->ObtenerY() + dy;

    int anchoJugadorVisual = this->jugador->ObtenerAnchoVisual();
    int altoJugadorVisual = this->jugador->ObtenerAltoVisual();
    int xEsquinaSiguiente = xSiguienteAnclaje - (anchoJugadorVisual / 2);
    int yEsquinaSiguiente = ySiguienteAnclaje;

    bool puedeMover = this->mapaActual->EsAreaValidaParaObjeto(xEsquinaSiguiente, yEsquinaSiguiente, anchoJugadorVisual, altoJugadorVisual);

    if (puedeMover) {
        this->jugador->Mover(dx, dy);

        array<ObjetoJuego^>^ todosLosObjetosEnMapa = this->mapaActual->ObtenerTodosObjetos();
        array<Recurso^>^ recursosParaProcesar = gcnew array<Recurso^>(todosLosObjetosEnMapa->Length);
        int countRecursos = 0;

        for each(ObjetoJuego ^ objEnMapa in todosLosObjetosEnMapa) {
            if (objEnMapa == nullptr || objEnMapa == this->jugador) continue;

            if (this->jugador->HaySuperposicionVisual(objEnMapa)) {
                Recurso^ recurso = dynamic_cast<Recurso^>(objEnMapa);
                if (recurso != nullptr && recurso->ObtenerTipoRecurso() != TipoRecurso::BLOQUE_CONSTRUIDO) {
                    if (countRecursos < recursosParaProcesar->Length)
                        recursosParaProcesar[countRecursos++] = recurso;
                }
                else {
                    Enemigo^ enemigo = dynamic_cast<Enemigo^>(objEnMapa);
                    if (enemigo != nullptr) {
                        this->jugador->RecibirDano(enemigo->ObtenerDano());
                    }
                }
            }
        }
        for (int i = 0; i < countRecursos; ++i) {
            Recurso^ recursoARecolectar = recursosParaProcesar[i];
            this->jugador->RecolectarRecurso(recursoARecolectar);
            this->mapaActual->EliminarObjeto(recursoARecolectar);

            this->ultimoRecursoNombre = recursoARecolectar->ObtenerNombre();
            this->ultimoRecursoColor = recursoARecolectar->ObtenerColorIcono();
            this->mostrarMensajeRecurso = true;

            this->ConstruirMundo3(recursoARecolectar);
        }
    }
}


void GestorJuego::IntentarInvocarAliado() {
    if ((this->estadoActual == GestorJuego::EstadoJuego::MUNDO1 || this->estadoActual == GestorJuego::EstadoJuego::MUNDO2) &&
        this->aliadosSpawneados < MAX_ALIADOS_SPAWN && this->mapaActual != nullptr && this->jugador != nullptr) {

        wchar_t caracterAliado = (this->estadoActual == GestorJuego::EstadoJuego::MUNDO1) ? L'F' : L'H';
        int velocidadAliado = (this->estadoActual == GestorJuego::EstadoJuego::MUNDO1) ?
            ((this->dificultadJuego == GestorJuego::Dificultad::FACIL) ? 3 : (this->dificultadJuego == GestorJuego::Dificultad::DIFICIL) ? 5 : 4) :
            ((this->dificultadJuego == GestorJuego::Dificultad::FACIL) ? 2 : (this->dificultadJuego == GestorJuego::Dificultad::DIFICIL) ? 4 : 3);

        Aliado^ aliadoPrototipo = gcnew Aliado(0, 0, ConsoleColor::Gray, velocidadAliado, caracterAliado);
        int spawnX = 0, spawnY = 0;
        bool posicionEncontrada = false;

        for (int i = 0; i < 20; ++i) {
            int offX = (rand() % 5) - 2; int offY = (rand() % 5) - 2;
            if (offX == 0 && offY == 0) offX = (rand() % 2 == 0) ? 1 : -1;

            int tempX = this->jugador->ObtenerX() + offX;
            int tempY = this->jugador->ObtenerY() + offY;
            if (this->GenerarPosicionValidaParaObjeto(this->mapaActual, aliadoPrototipo, tempX, tempY, 1)) {
                spawnX = tempX;
                spawnY = tempY;
                posicionEncontrada = true;
                break;
            }
        }
        if (!posicionEncontrada) {
            if (this->GenerarPosicionValidaParaObjeto(this->mapaActual, aliadoPrototipo, spawnX, spawnY)) {
                posicionEncontrada = true;
            }
        }

        if (posicionEncontrada) {
            Aliado^ nuevoAliado = gcnew Aliado(spawnX, spawnY, (caracterAliado == L'F' ? ConsoleColor::Green : ConsoleColor::DarkCyan), velocidadAliado, caracterAliado);
            this->mapaActual->AgregarObjeto(nuevoAliado); this->aliadosSpawneados++;
            try {
                int msgPosY = this->altoConsola;
                if (this->estadoActual == GestorJuego::EstadoJuego::VISTA_MUNDO3) {
                    msgPosY = this->altoConsola - 2;
                }
                else {
                    msgPosY = this->altoConsola;
                }

                if (msgPosY < 1 && this->altoConsola >= 1) msgPosY = 1;
                else if (msgPosY < 1) msgPosY = 0;
                if (msgPosY > this->altoConsola) msgPosY = this->altoConsola;


                this->LimpiarAreaConsola(1, msgPosY, Math::Max(0, this->anchoConsola - 2), 1);
                Console::SetCursorPosition(1, msgPosY);
                Console::ForegroundColor = ConsoleColor::Yellow;
                Console::Write("Aliado " + caracterAliado + " invocado! Restantes: " + (MAX_ALIADOS_SPAWN - this->aliadosSpawneados));
                Console::ResetColor();
            }
            catch (Exception^) {}
        }
        aliadoPrototipo = nullptr;
    }
}

void GestorJuego::ActualizarJuego() {
    if (this->mapaActual && this->jugador && (this->estadoActual == GestorJuego::EstadoJuego::MUNDO1 || this->estadoActual == GestorJuego::EstadoJuego::MUNDO2)) {
        this->ActualizarMundo(this->mapaActual);
    }
}

void GestorJuego::ActualizarMundo(Mapa^ mapaAActualizar) {
    if (mapaAActualizar && this->jugador) mapaAActualizar->ActualizarTodos(this->jugador);
}

void GestorJuego::RenderizarMenuPrincipal() {
    this->DibujarMenu(this->elementosMenuPrincipal, this->elementosMenuPrincipal->Length, this->opcionMenuPrincipalSel, "=== VIGILANTE DEL EQUILIBRIO ===");
}

void GestorJuego::RenderizarMenuDificultad() {
    this->DibujarMenu(this->elementosMenuDificultad, this->elementosMenuDificultad->Length, -1, "Selecciona Dificultad");
}

void GestorJuego::RenderizarPantallaInstrucciones() {
    int izq = Math::Max(1, this->anchoConsola / 10);
    int arr = Math::Max(1, this->altoConsola / 8);
    int maxAnchoTexto = this->anchoConsola - (izq * 2);
    if (maxAnchoTexto < 20) maxAnchoTexto = Math::Min(this->anchoConsola - 2, 20);
    if (maxAnchoTexto <= 0) maxAnchoTexto = 1;
    try {
        Console::ForegroundColor = ConsoleColor::Cyan;
        Console::SetCursorPosition(izq, arr);
        Console::Write("--- INSTRUCCIONES DEL JUEGO ---");
        Console::ForegroundColor = ConsoleColor::White;
        array<String^>^ lineas = gcnew array<String^>{
            "- Usa W/A/S/D o las Flechas para moverte.",
            "- Mundo 1 (Humanitas): Recolecta Habilidades Humanas.",
            "- Mundo 2 (Technos): Recolecta Recursos Tecnologicos.",
            "- Esquiva a los Enemigos para no perder vidas.",
            "- Invoca Aliados con la tecla 'H' (max " + MAX_ALIADOS_SPAWN + ").",
            "- Lleva recursos/habilidades al Mundo 3 (presiona '3').",
            "- Objetivo: " + META_PORCENTAJE_MUNDO3 + "% de progreso en Mundo 3.",
            "- Tiempo limite: " + (int)(LIMITE_TIEMPO_JUEGO_SEGUNDOS / 60) + " minutos.",
            "- Teclas '1', '2', '3' para cambiar entre mundos. ESC para Menu."
        };
        int yActual = arr + 2;
        for each(String ^ linea in lineas) {
            if (yActual >= this->altoConsola) break;
            int charActual = 0;
            while (charActual < linea->Length) {
                int longitudTomar = Math::Min(maxAnchoTexto, linea->Length - charActual);
                String^ subLinea = linea->Substring(charActual, longitudTomar);
                if (izq >= 0 && izq < Console::BufferWidth && yActual >= 0 && yActual < Console::BufferHeight) {
                    Console::SetCursorPosition(izq, yActual++); Console::Write(subLinea);
                }
                else { break; }
                charActual += longitudTomar;
                if (yActual >= this->altoConsola) break;
            }
        }
        int msgYPos = this->altoConsola;
        if (msgYPos < 0) msgYPos = 0;
        if (msgYPos >= Console::BufferHeight && Console::BufferHeight > 0) msgYPos = Console::BufferHeight - 1;

        Console::ForegroundColor = ConsoleColor::Yellow;
        if (izq >= 0 && izq < Console::BufferWidth && msgYPos >= 0 && msgYPos < Console::BufferHeight) {
            this->LimpiarAreaConsola(0, msgYPos, this->anchoConsola, 1);
            Console::SetCursorPosition(izq, msgYPos);
            Console::Write("Presiona ESC para volver al menu principal.");
        }
        Console::ResetColor();
    }
    catch (Exception^) {}
}

void GestorJuego::RenderizarPantallaIntegrantes() {
    int izq = Math::Max(1, this->anchoConsola / 10); int arr = Math::Max(1, this->altoConsola / 8);
    try {
        Console::ForegroundColor = ConsoleColor::Cyan;
        Console::SetCursorPosition(izq, arr); Console::Write("--- INTEGRANTES DEL EQUIPO ---");
        Console::ForegroundColor = ConsoleColor::White;
        array<String^>^ integrantes = {
            "- Piero Benjamin Ayvar Valdez  - u202312035",
            "- Evelyn Milene Chavez Valeriano - u20241c489",
            "- Sebastian Alexander Ramos Calagua - u202222846"
        };
        for (int i = 0; i < integrantes->Length; ++i) {
            if (arr + 2 + i < this->altoConsola) {
                Console::SetCursorPosition(izq, arr + 2 + i); Console::Write(integrantes[i]);
            }
            else break;
        }
        int msgYPos = this->altoConsola;
        if (msgYPos < 0) msgYPos = 0;
        if (msgYPos >= Console::BufferHeight && Console::BufferHeight > 0) msgYPos = Console::BufferHeight - 1;

        Console::ForegroundColor = ConsoleColor::Yellow;
        if (izq >= 0 && izq < Console::BufferWidth && msgYPos >= 0 && msgYPos < Console::BufferHeight) {
            this->LimpiarAreaConsola(0, msgYPos, this->anchoConsola, 1);
            Console::SetCursorPosition(izq, msgYPos);
            Console::Write("Presiona ESC para volver al menu principal.");
        }
        Console::ResetColor();
    }
    catch (Exception^) {}
}

void GestorJuego::RenderizarPantallaCreditos() {
    int izq = Math::Max(1, this->anchoConsola / 10); int arr = Math::Max(1, this->altoConsola / 8);
    try {
        Console::ForegroundColor = ConsoleColor::Cyan;
        Console::SetCursorPosition(izq, arr); Console::Write("--- CREDITOS ---");
        Console::ForegroundColor = ConsoleColor::White;
        array<String^>^ creditos = {
            "- Juego desarrollado para el curso Algoritmos (1ACC215)",
            "- Docente: Luis Alberto Raymundo Chacaltana",
            "- Institucion: Universidad Peruana de Ciencias Aplicadas (UPC)",
            "- Lenguaje: C++/CLR (.NET Framework)",
            "- Fecha de Entrega: Mayo 2025"
        };
        for (int i = 0; i < creditos->Length; ++i) {
            if (arr + 2 + i < this->altoConsola) {
                Console::SetCursorPosition(izq, arr + 2 + i); Console::Write(creditos[i]);
            }
            else break;
        }
        int msgYPos = this->altoConsola;
        if (msgYPos < 0) msgYPos = 0;
        if (msgYPos >= Console::BufferHeight && Console::BufferHeight > 0) msgYPos = Console::BufferHeight - 1;

        Console::ForegroundColor = ConsoleColor::Yellow;
        if (izq >= 0 && izq < Console::BufferWidth && msgYPos >= 0 && msgYPos < Console::BufferHeight) {
            this->LimpiarAreaConsola(0, msgYPos, this->anchoConsola, 1);
            Console::SetCursorPosition(izq, msgYPos);
            Console::Write("Presiona ESC para volver al menu principal.");
        }
        Console::ResetColor();
    }
    catch (Exception^) {}
}

void GestorJuego::RenderizarMundo() {
    if (this->mapaActual != nullptr) {
        this->mapaActual->Dibujar();
    }
    else {
        try { Console::SetCursorPosition(5, 5); Console::Write("Error: Mapa no cargado."); }
        catch (Exception^) {}
    }
}

void GestorJuego::RenderizarVistaMundo3() {
    if (this->mapaMundo3 == nullptr) return;
    this->mapaMundo3->Dibujar();

    String^ barraProgresoVisual = "[";
    int llenadoBarra = static_cast<int>(Math::Round(static_cast<double>(this->progresoMundo3) / 100.0 * 20.0));
    for (int i = 0; i < 20; ++i) {
        barraProgresoVisual += (i < llenadoBarra) ? L'#' : L'-';
    }
    barraProgresoVisual += "]";
    String^ textoProgreso = "Progreso Construccion Mundo 3: " + this->progresoMundo3 + "% " + barraProgresoVisual;

    int posYProgreso = this->altoConsola - 1;
    int posXProgreso = Math::Max(1, (this->anchoConsola - textoProgreso->Length) / 2);
    if (posXProgreso + textoProgreso->Length >= this->anchoConsola && this->anchoConsola > 0) {
        posXProgreso = Math::Max(0, this->anchoConsola - textoProgreso->Length - 1);
    }
    if (posXProgreso < 0) posXProgreso = 0;

    try {
        Console::BackgroundColor = ConsoleColor::Black;
        Console::ForegroundColor = ConsoleColor::Yellow;
        this->LimpiarAreaConsola(0, posYProgreso, this->anchoConsola, 1);
        if (posYProgreso >= 0 && posYProgreso < Console::BufferHeight && posXProgreso >= 0 && posXProgreso + textoProgreso->Length <= Console::BufferWidth) {
            Console::SetCursorPosition(posXProgreso, posYProgreso);
            Console::Write(textoProgreso);
        }
    }
    catch (Exception^) {}

    String^ msgVolver = "Presiona 1 (Humanitas) o 2 (Technos) para volver | ESC para Menu";
    int posYVolver = this->altoConsola;
    int posXVolver = Math::Max(1, (this->anchoConsola - msgVolver->Length) / 2);
    if (posXVolver + msgVolver->Length >= this->anchoConsola && this->anchoConsola > 0) {
        posXVolver = Math::Max(0, this->anchoConsola - msgVolver->Length - 1);
    }
    if (posXVolver < 0) posXVolver = 0;

    try {
        Console::BackgroundColor = ConsoleColor::Black;
        Console::ForegroundColor = ConsoleColor::DarkGray;
        this->LimpiarAreaConsola(0, posYVolver, this->anchoConsola, 1);
        if (posYVolver >= 0 && posYVolver < Console::BufferHeight && posXVolver >= 0 && posXVolver + msgVolver->Length <= Console::BufferWidth) {
            Console::SetCursorPosition(posXVolver, posYVolver);
            Console::Write(msgVolver);
        }
        Console::ResetColor();
    }
    catch (Exception^) {}
}

void GestorJuego::RenderizarPantallaCarga() {
    Console::BackgroundColor = ConsoleColor::Black;
    try { Console::Clear(); }
    catch (Exception^) {}

    String^ textoCarga = "CARGANDO " + (this->nombreMundoCargando != nullptr ? this->nombreMundoCargando->ToUpper() : "MUNDO DESCONOCIDO") + "...";
    int yTexto = Math::Max(1, this->altoConsola / 2 - 1);
    int xTexto = Math::Max(0, (this->anchoConsola - textoCarga->Length) / 2);

    try {
        Console::ForegroundColor = ConsoleColor::Cyan;
        if (yTexto >= 0 && yTexto < Console::BufferHeight && xTexto >= 0 && xTexto + textoCarga->Length <= Console::BufferWidth) {
            Console::SetCursorPosition(xTexto, yTexto);
            Console::Write(textoCarga);
        }

        int yBarra = yTexto + 2;
        int anchoBarraTotalDisplay = Math::Min(50, this->anchoConsola - 20);
        if (anchoBarraTotalDisplay < 10) anchoBarraTotalDisplay = 10;

        String^ porcentajeTexto = " " + this->progresoCargaActual + "%";
        int longitudTotalBarraConPorcentaje = anchoBarraTotalDisplay + 2 + porcentajeTexto->Length;
        int xBarra = Math::Max(0, (this->anchoConsola - longitudTotalBarraConPorcentaje) / 2);

        String^ barraVisual = "[";
        int caracteresLlenos = static_cast<int>(Math::Round((static_cast<double>(this->progresoCargaActual) / 100.0) * anchoBarraTotalDisplay));

        for (int i = 0; i < anchoBarraTotalDisplay; ++i) {
            barraVisual += (i < caracteresLlenos) ? L'#' : L'-';
        }
        barraVisual += "]" + porcentajeTexto;

        Console::ForegroundColor = ConsoleColor::Green;
        if (yBarra >= 0 && yBarra < Console::BufferHeight && xBarra >= 0 && xBarra + barraVisual->Length <= Console::BufferWidth) {
            Console::SetCursorPosition(xBarra, yBarra);
            Console::Write(barraVisual);
        }
        Console::ResetColor();
    }
    catch (Exception^) {}
}


void GestorJuego::RenderizarFinJuego() {
    Console::BackgroundColor = ConsoleColor::Black;
    try { Console::Clear(); }
    catch (Exception^) {}
    this->MostrarFinJuegoInterno(this->estadoActual == GestorJuego::EstadoJuego::FIN_JUEGO_VICTORIA);
}

void GestorJuego::IniciarJuegoNuevo(Dificultad dificultadSeleccionada) {
    this->dificultadJuego = dificultadSeleccionada;
    this->InicializarDatosJuego();
    this->InicializarMundosYJugador();
    this->estadoActualAnterior = this->estadoActual;
    this->estadoActual = GestorJuego::EstadoJuego::MUNDO1;
    this->tiempoInicio = time(0);
    this->necesitaRedibujarCompleto = true;
    if (!this->juegoEnCurso) this->juegoEnCurso = true;
}

void GestorJuego::CambiarMundo(EstadoJuego estadoMundoDestinoDeseado) {
    if (this->jugador == nullptr || this->mapaMundo1 == nullptr || this->mapaMundo2 == nullptr || this->mapaMundo3 == nullptr) return;
    if (this->mapaActual != nullptr) {
        if (this->estadoActual == estadoMundoDestinoDeseado) return;
    }

    this->estadoActualAnterior = this->estadoActual;

    switch (estadoMundoDestinoDeseado) {
    case GestorJuego::EstadoJuego::MUNDO1:
        this->nombreMundoCargando = (this->mapaMundo1 != nullptr) ? this->mapaMundo1->ObtenerNombre() : "Mundo 1";
        break;
    case GestorJuego::EstadoJuego::MUNDO2:
        this->nombreMundoCargando = (this->mapaMundo2 != nullptr) ? this->mapaMundo2->ObtenerNombre() : "Mundo 2";
        break;
    case GestorJuego::EstadoJuego::VISTA_MUNDO3:
        this->nombreMundoCargando = (this->mapaMundo3 != nullptr) ? this->mapaMundo3->ObtenerNombre() : "Mundo 3 (Vista)";
        break;
    default:
        this->estadoActual = GestorJuego::EstadoJuego::MENU_PRINCIPAL;
        this->necesitaRedibujarCompleto = true;
        return;
    }

    this->estadoMundoSiguiente = estadoMundoDestinoDeseado;
    this->estadoActual = GestorJuego::EstadoJuego::PANTALLA_CARGA;
    this->progresoCargaActual = 0;
    this->tiempoInicioCarga = clock();
    this->necesitaRedibujarCompleto = true;
}


void GestorJuego::ConstruirMundo3(Recurso^ recursoRecolectado) {
    if (recursoRecolectado == nullptr || this->mapaMundo3 == nullptr) return;
    if (recursoRecolectado->ObtenerTipoRecurso() != TipoRecurso::BLOQUE_CONSTRUIDO &&
        recursoRecolectado->ObtenerValor() > 0) {
        this->progresoMundo3 += recursoRecolectado->ObtenerValor();
        if (this->progresoMundo3 > 100) this->progresoMundo3 = 100;

        Recurso^ bloquePrototipo = gcnew Recurso(0, 0, ConsoleColor::Gray, TipoRecurso::BLOQUE_CONSTRUIDO, "B", 0, "[#]", nullptr);

        int xBloque, yBloque;
        if (this->GenerarPosicionValidaParaObjeto(this->mapaMundo3, bloquePrototipo, xBloque, yBloque, 50)) {
            ConsoleColor colorBloque = ConsoleColor::DarkGray;
            String^ rep1Bloque = "[B]";
            String^ rep2Bloque = nullptr;

            TipoRecurso tipoOriginal = recursoRecolectado->ObtenerTipoRecurso();
            if (tipoOriginal >= TipoRecurso::HABILIDAD_HUMANA_EMPATIA && tipoOriginal <= TipoRecurso::HABILIDAD_HUMANA_CREATIVIDAD) {
                colorBloque = ConsoleColor::DarkGreen;
                rep1Bloque = "/WWW\\";
                rep2Bloque = "| H |";
            }
            else if (tipoOriginal >= TipoRecurso::TECNOLOGIA_IA && tipoOriginal <= TipoRecurso::TECNOLOGIA_AUTOMATIZACION) {
                colorBloque = ConsoleColor::DarkBlue;
                rep1Bloque = "#####";
                rep2Bloque = "# T #";
            }
            Recurso^ bloqueConstruido = gcnew Recurso(xBloque, yBloque, colorBloque, TipoRecurso::BLOQUE_CONSTRUIDO, "BloqueM3", 0, rep1Bloque, rep2Bloque);
            this->mapaMundo3->AgregarObjeto(bloqueConstruido);
        }
        bloquePrototipo = nullptr;
        this->VerificarCondicionesFin();
    }
}

void GestorJuego::VerificarCondicionesFin() {
    if (this->jugador == nullptr || !this->juegoEnCurso) return;
    bool condicionFinAlcanzada = false;
    if (this->jugador->ObtenerVidas() <= 0) {
        this->estadoActual = GestorJuego::EstadoJuego::FIN_JUEGO_DERROTA; condicionFinAlcanzada = true;
    }
    time_t tiempoActual = time(0);
    double tiempoTranscurrido = (this->tiempoInicio == 0) ? 0.0 : difftime(tiempoActual, this->tiempoInicio);
    if (!condicionFinAlcanzada && tiempoTranscurrido >= LIMITE_TIEMPO_JUEGO_SEGUNDOS && this->tiempoInicio != 0) {
        this->estadoActual = GestorJuego::EstadoJuego::FIN_JUEGO_DERROTA; condicionFinAlcanzada = true;
    }
    if (!condicionFinAlcanzada && this->progresoMundo3 >= META_PORCENTAJE_MUNDO3) {
        this->estadoActual = GestorJuego::EstadoJuego::FIN_JUEGO_VICTORIA; condicionFinAlcanzada = true;
    }
    if (condicionFinAlcanzada) {
        this->juegoEnCurso = false;
        this->necesitaRedibujarCompleto = true;
    }
}

void GestorJuego::MostrarHUD() {
    if (this->jugador == nullptr) return;
    double tiempoTranscurrido = (this->tiempoInicio == 0) ? 0.0 : difftime(time(0), this->tiempoInicio);
    double tiempoRestanteSegundos = Math::Max(0.0, LIMITE_TIEMPO_JUEGO_SEGUNDOS - tiempoTranscurrido);
    int minutos = static_cast<int>(tiempoRestanteSegundos / 60);
    int segundos = static_cast<int>(tiempoRestanteSegundos) % 60;

    String^ vidasStr = "Vidas: " + this->jugador->ObtenerVidas();
    String^ tiempoStr = String::Format("Tiempo: {0:D2}:{1:D2}", minutos, segundos);
    String^ progresoStr = (this->estadoActual == GestorJuego::EstadoJuego::VISTA_MUNDO3 || this->progresoMundo3 > 0) ? "M3: " + this->progresoMundo3 + "%" : "";
    String^ mundoNombre = (this->mapaActual != nullptr) ? this->mapaActual->ObtenerNombre() : "N/A";
    String^ mundoStr = "Mapa: " + mundoNombre->Substring(0, Math::Min(mundoNombre->Length, 10));
    String^ aliadosStr = "Aliados: " + this->aliadosSpawneados + "/" + MAX_ALIADOS_SPAWN;
    String^ separador = " | ";

    String^ lineaHUD = vidasStr + separador + tiempoStr + separador;
    if (!String::IsNullOrEmpty(progresoStr)) lineaHUD += progresoStr + separador;
    lineaHUD += mundoStr + separador + aliadosStr;

    int hudWidth = Math::Max(0, this->anchoConsola);
    String^ hudPadding = gcnew String(L' ', Math::Max(0, hudWidth - lineaHUD->Length));
    lineaHUD += hudPadding;
    if (lineaHUD->Length > hudWidth && hudWidth > 0) lineaHUD = lineaHUD->Substring(0, hudWidth);

    ConsoleColor bgAnterior = Console::BackgroundColor;
    ConsoleColor fgAnterior = Console::ForegroundColor;
    try {
        Console::ForegroundColor = ConsoleColor::White;
        Console::BackgroundColor = ConsoleColor::DarkMagenta;
        if (0 < Console::BufferWidth && 0 < Console::BufferHeight) {
            Console::SetCursorPosition(0, 0);
            Console::Write(lineaHUD);
        }
    }
    catch (Exception^) {}
    finally {
        Console::BackgroundColor = bgAnterior;
        Console::ForegroundColor = fgAnterior;
    }
}

void GestorJuego::MostrarInventarioYRecoleccionHUD() {
    int posYMensajes = this->altoConsola;
    if (posYMensajes < 1 && this->altoConsola >= 1) posYMensajes = 1;
    else if (posYMensajes < 1) posYMensajes = 0;
    if (posYMensajes >= Console::BufferHeight) posYMensajes = Console::BufferHeight > 0 ? Console::BufferHeight - 1 : 0;

    this->LimpiarAreaConsola(0, posYMensajes, this->anchoConsola, 1);

    int posXActual = 1;
    if (posYMensajes >= 0 && posYMensajes < Console::BufferHeight) {
        try { Console::SetCursorPosition(posXActual, posYMensajes); }
        catch (Exception^) { return; }
    }
    else {
        return;
    }

    if (this->mostrarMensajeRecurso && this->ultimoRecursoNombre != nullptr &&
        (this->estadoActual == GestorJuego::EstadoJuego::MUNDO1 || this->estadoActual == GestorJuego::EstadoJuego::MUNDO2)) {
        String^ mensajeRecoleccion = "Recolectado: ";
        Console::ForegroundColor = ConsoleColor::White;
        Console::Write(mensajeRecoleccion);
        posXActual += mensajeRecoleccion->Length;

        Console::ForegroundColor = this->ultimoRecursoColor;
        Console::Write(this->ultimoRecursoNombre);
        posXActual += this->ultimoRecursoNombre->Length;

        Console::ForegroundColor = ConsoleColor::White;
        String^ separadorInventario = " -> Inv: ";
        Console::Write(separadorInventario);
        posXActual += separadorInventario->Length;
    }

    if (this->jugador != nullptr && (this->estadoActual == GestorJuego::EstadoJuego::MUNDO1 || this->estadoActual == GestorJuego::EstadoJuego::MUNDO2)) {
        Dictionary<int, int>^ conteoRecursos = gcnew Dictionary<int, int>();
        Dictionary<int, Recurso^>^ muestraRecursos = gcnew Dictionary<int, Recurso^>();

        array<Recurso^>^ inventarioActual = this->jugador->ObtenerInventario();
        for each(Recurso ^ rec in inventarioActual) {
            if (rec == nullptr) continue;
            int tipoRecursoInt = static_cast<int>(rec->ObtenerTipoRecurso());
            if (conteoRecursos->ContainsKey(tipoRecursoInt)) {
                conteoRecursos[tipoRecursoInt] = conteoRecursos[tipoRecursoInt] + 1;
            }
            else {
                conteoRecursos->Add(tipoRecursoInt, 1);
                muestraRecursos->Add(tipoRecursoInt, rec);
            }
        }

        if (posYMensajes >= 0 && posYMensajes < Console::BufferHeight) {
            if (conteoRecursos->Count > 0) {
                if (!this->mostrarMensajeRecurso || this->ultimoRecursoNombre == nullptr) {
                    String^ etiquetaInventario = "Inventario: ";
                    if (posXActual + etiquetaInventario->Length < this->anchoConsola) {
                        try { Console::SetCursorPosition(posXActual, posYMensajes); }
                        catch (Exception^) { Console::ResetColor(); return; }
                        Console::ForegroundColor = ConsoleColor::Gray;
                        Console::Write(etiquetaInventario);
                        posXActual += etiquetaInventario->Length;
                    }
                }

                for each(KeyValuePair<int, int> par in conteoRecursos) {
                    if (!muestraRecursos->ContainsKey(par.Key)) continue;

                    Recurso^ muestra = muestraRecursos[par.Key];
                    String^ miniAscii = muestra->ObtenerMiniAscii();
                    ConsoleColor colorIcono = muestra->ObtenerColorIcono();
                    String^ itemStr = String::Format("{0}x{1} ", miniAscii, par.Value);

                    if (posXActual + itemStr->Length < this->anchoConsola) {
                        try { Console::SetCursorPosition(posXActual, posYMensajes); }
                        catch (Exception^) { Console::ResetColor(); return; }
                        Console::ForegroundColor = colorIcono;
                        Console::Write(miniAscii);

                        Console::ForegroundColor = ConsoleColor::White;
                        Console::Write("x" + par.Value + " ");
                        posXActual += itemStr->Length;
                    }
                    else {
                        if (posXActual < this->anchoConsola && posYMensajes >= 0 && posYMensajes < Console::BufferHeight) {
                            try { Console::SetCursorPosition(posXActual, posYMensajes); }
                            catch (Exception^) { Console::ResetColor(); return; }
                            Console::Write("...");
                        }
                        break;
                    }
                }
            }
            else if (!this->mostrarMensajeRecurso || this->ultimoRecursoNombre == nullptr) {
                if (posXActual < this->anchoConsola - 15 && posYMensajes >= 0 && posYMensajes < Console::BufferHeight) {
                    try { Console::SetCursorPosition(posXActual, posYMensajes); }
                    catch (Exception^) { Console::ResetColor(); return; }
                    Console::ForegroundColor = ConsoleColor::DarkGray;
                    Console::Write("Inventario vacio...");
                }
            }
        }
    }
    Console::ResetColor();
}


void GestorJuego::LimpiarAreaConsola(int izq, int arr, int ancho, int altoParam) {
    if (izq < 0) izq = 0; if (arr < 0) arr = 0;
    if (ancho <= 0 || altoParam <= 0) return;

    ancho = Math::Min(ancho, Console::BufferWidth - izq);
    altoParam = Math::Min(altoParam, Console::BufferHeight - arr);
    if (ancho <= 0 || altoParam <= 0) return;

    String^ lineaLimpiar = gcnew String(L' ', ancho);
    ConsoleColor bgOriginal = Console::BackgroundColor;
    ConsoleColor fgOriginal = Console::ForegroundColor;
    Console::BackgroundColor = ConsoleColor::Black;
    Console::ForegroundColor = ConsoleColor::Black; // Para "pintar" espacios negros

    for (int y = arr; y < arr + altoParam; ++y) {
        if (y >= 0 && y < Console::BufferHeight && izq >= 0 && izq + ancho <= Console::BufferWidth) {
            try {
                Console::SetCursorPosition(izq, y);
                Console::Write(lineaLimpiar);
            }
            catch (Exception^) { break; }
        }
        else {
            if (y >= Console::BufferHeight) break;
        }
    }
    Console::BackgroundColor = bgOriginal;
    Console::ForegroundColor = fgOriginal;
}

void GestorJuego::MostrarFinJuegoInterno(bool jugadorGano) {
    String^ mensaje1 = jugadorGano ? "*** FELICIDADES, HAS GANADO! ***" : "--- GAME OVER ---";
    String^ mensaje2 = jugadorGano ? "Lograste el equilibrio y construiste el Mundo 3!" :
        (this->jugador != nullptr && this->jugador->ObtenerVidas() <= 0 ? "Te quedaste sin vidas." : "Se acabo el tiempo o no alcanzaste el objetivo.");

    int msgYBase = Math::Max(1, this->altoConsola / 2 - 2);
    if (msgYBase < 0) msgYBase = 0;

    int msg1X = Math::Max(0, (this->anchoConsola - mensaje1->Length) / 2);
    int msg2X = Math::Max(0, (this->anchoConsola - mensaje2->Length) / 2);
    if (msg1X < 0) msg1X = 0; if (msg2X < 0) msg2X = 0;

    try {
        Console::ForegroundColor = jugadorGano ? ConsoleColor::Green : ConsoleColor::Red;
        Console::BackgroundColor = ConsoleColor::Black;
        if (msgYBase >= 0 && msgYBase < Console::BufferHeight && msg1X >= 0 && msg1X + mensaje1->Length <= Console::BufferWidth) {
            Console::SetCursorPosition(msg1X, msgYBase); Console::Write(mensaje1);
        }
        Console::ForegroundColor = ConsoleColor::White;
        if (msgYBase + 1 >= 0 && msgYBase + 1 < Console::BufferHeight && msg2X >= 0 && msg2X + mensaje2->Length <= Console::BufferWidth) {
            Console::SetCursorPosition(msg2X, msgYBase + 1); Console::Write(mensaje2);
        }
        Console::ResetColor();
    }
    catch (Exception^) {}
}

void GestorJuego::DibujarMenu(array<String^>^ elementosMenu, int cantidadElementos, int indiceOpcionSel, String^ titulo) {
    int yInicio = Math::Max(2, this->altoConsola / 3);
    if (yInicio < 0) yInicio = 0;
    int xInicioTitulo = Math::Max(0, (this->anchoConsola - titulo->Length) / 2);
    if (xInicioTitulo < 0) xInicioTitulo = 0;

    try {
        Console::ForegroundColor = ConsoleColor::Yellow;
        Console::BackgroundColor = ConsoleColor::Black;
        if (yInicio - 2 >= 0 && yInicio - 2 < Console::BufferHeight && xInicioTitulo >= 0 && xInicioTitulo + titulo->Length <= Console::BufferWidth) {
            Console::SetCursorPosition(xInicioTitulo, Math::Max(0, yInicio - 2));
            Console::Write(titulo);
        }

        for (int i = 0; i < cantidadElementos; ++i) {
            if (yInicio + i >= this->altoConsola - 3 && this->altoConsola > 3) break;

            String^ textoElementoOriginal = elementosMenu[i];
            String^ textoADibujar = textoElementoOriginal;
            ConsoleColor colorFg = ConsoleColor::White;
            ConsoleColor colorBg = ConsoleColor::Black;

            bool estaSeleccionado = false;
            if (indiceOpcionSel == -1) {
                if (textoElementoOriginal->StartsWith(">") && textoElementoOriginal->EndsWith("<")) {
                    estaSeleccionado = true;
                }
            }
            else {
                estaSeleccionado = (i == indiceOpcionSel);
            }

            if (estaSeleccionado) {
                if (indiceOpcionSel != -1) textoADibujar = ">> " + textoElementoOriginal->Trim() + " <<";
                colorFg = ConsoleColor::Black; colorBg = ConsoleColor::White;
            }
            else {
                if (indiceOpcionSel != -1) textoADibujar = "   " + textoElementoOriginal->Trim() + "   ";
            }

            int xInicioItem = Math::Max(0, (this->anchoConsola - textoADibujar->Length) / 2);
            if (xInicioItem < 0) xInicioItem = 0;

            if (yInicio + i >= 0 && yInicio + i < Console::BufferHeight && xInicioItem >= 0 && xInicioItem + textoADibujar->Length <= Console::BufferWidth) {
                Console::SetCursorPosition(xInicioItem, yInicio + i);
                Console::ForegroundColor = colorFg; Console::BackgroundColor = colorBg;
                Console::Write(textoADibujar);
            }
            else { break; }
        }
        Console::ResetColor();

        Console::ForegroundColor = ConsoleColor::DarkGray;
        Console::BackgroundColor = ConsoleColor::Black;
        String^ navMsg1 = (indiceOpcionSel != -1) ? "Navega: W/S o Flechas | Selecciona: Enter" : "Navega: A/D o Izq/Der | Selecciona: Enter";
        String^ navMsg2 = "Volver/Salir: ESC";

        int navY1 = (yInicio + cantidadElementos + 2 < this->altoConsola - 1 && this->altoConsola > 1) ? yInicio + cantidadElementos + 2 : (this->altoConsola > 1 ? this->altoConsola - 2 : 0);
        if (navY1 < 0) navY1 = 0;
        int navY2 = (navY1 + 1 < this->altoConsola && this->altoConsola > 0) ? navY1 + 1 : (this->altoConsola > 0 ? this->altoConsola - 1 : 0);
        if (navY2 < 0) navY2 = 0;

        if (navY1 >= navY2 && navY2 > 0) navY1 = navY2 - 1; if (navY1 < 0) navY1 = 0;

        int nav1X = Math::Max(0, (this->anchoConsola - navMsg1->Length) / 2); if (nav1X < 0)nav1X = 0;
        int nav2X = Math::Max(0, (this->anchoConsola - navMsg2->Length) / 2); if (nav2X < 0)nav2X = 0;

        if (navY1 >= 0 && navY1 < Console::BufferHeight && nav1X >= 0 && nav1X + navMsg1->Length <= Console::BufferWidth) {
            this->LimpiarAreaConsola(0, navY1, this->anchoConsola, 1);
            Console::SetCursorPosition(nav1X, navY1); Console::Write(navMsg1);
        }
        if (navY2 >= 0 && navY2 < Console::BufferHeight && nav2X >= 0 && nav2X + navMsg2->Length <= Console::BufferWidth) {
            this->LimpiarAreaConsola(0, navY2, this->anchoConsola, 1);
            Console::SetCursorPosition(nav2X, navY2); Console::Write(navMsg2);
        }
        Console::ResetColor();
    }
    catch (Exception^) {}
}
// Esta es la última llave del archivo GestorJuego.cpp