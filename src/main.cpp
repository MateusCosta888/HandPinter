#include "Application.h"
#include <iostream>
#include <string>
#include <windows.h>

// ============================================================
// Ponto de entrada do programa HandPaint
// ============================================================

int main(int argc, char* argv[]) {
    // Forçar o Windows a destruir ou invisibilizar a Consola Negra 
    // que acorda invariavelmente (quando iniciamos via .bat ou explorador não-standalone)
    ::FreeConsole();

    // Caminho para os modelos TFLite (do repositório clonado)
    std::string caminhoModelos = "../mediapipe_hand_tracking_cpp/models";

    // Verificar se o utilizador especificou um caminho diferente
    if (argc > 1) {
        caminhoModelos = argv[1];
    }

    // Criar e executar a aplicação
    Application app(caminhoModelos);
    app.executar();

    return 0;
}
