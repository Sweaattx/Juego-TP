#include "GestorJuego.h"
using namespace System;
[STAThreadAttribute]
int main(array<System::String^>^ args) {
    GestorJuego^ gestorJuego = nullptr;
    try {
        gestorJuego = gcnew GestorJuego();
        gestorJuego->Ejecutar();
    }
    catch (System::Exception^ ex) {
        try {
            Console::ResetColor(); Console::Clear();
            Console::ForegroundColor = ConsoleColor::Red;
            Console::WriteLine("\n!!! EXCEPCION FATAL EN main() !!!");
            Console::WriteLine("Tipo: " + ex->GetType()->FullName);
            Console::WriteLine("Mensaje: " + ex->Message);
            Console::WriteLine("StackTrace:\n" + ex->StackTrace);
            Console::ResetColor();
            Console::WriteLine("\nEl programa terminara. Presiona cualquier tecla para cerrar...");
            Console::ReadKey(true);
        }
        catch (Exception^) {} return 1;
    }
    catch (...) {
        try {
            Console::ResetColor(); Console::Clear();
            Console::ForegroundColor = ConsoleColor::Red;
            Console::WriteLine("\n!!! EXCEPCION DESCONOCIDA FATAL EN main() !!!");
            Console::ResetColor();
            Console::WriteLine("\nEl programa terminara. Presiona cualquier tecla para cerrar...");
            Console::ReadKey(true);
        }
        catch (Exception^) {} return 1;
    } return 0;
}