#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include "Menu.h"
#include "HandTracker.h"
#include "Canvas.h"
#include "UIOverlay.h"

// ============================================================
// Classe Application (Classe Principal)
// Orquestra todos os componentes do programa:
// - Menu → HandTracker → Canvas → UIOverlay
// Relação: Composição (Application possui HandTracker*, Canvas, UIOverlay)
// ============================================================
class Application {
private:
    // Estado da aplicação
    enum Estado {
        MENU,
        PINTURA,
        A_SAIR
    };

    Estado estadoAtual;

    // Componentes (Composição)
    Menu menu;
    HandTracker* tracker;      // Apontador! (new/delete para cumprir requisito 4)
    Canvas* canvas;            // Apontador! (new/delete)
    UIOverlay* overlay;        // Apontador! (new/delete)

    // Câmara
    cv::VideoCapture camera;
    int cameraId;

    // Caminho para os modelos do hand tracking
    std::string caminhoModelos;

    // Dimensões da janela
    int larguraJanela;
    int alturaJanela;

    // Controlo de pintura
    bool estavaAPintar;        // Se na frame anterior o utilizador estava com a caneta
    bool estavaAApagar;        // Se na frame anterior o utilizador estava a usar a borracha

    // --- Métodos privados ---
    void inicializarComponentes();
    void libertarComponentes();
    void cicloPintura();
    void processarInput(int tecla);

    // Pedir ao utilizador o nome do ficheiro para guardar (via consola)
    std::string pedirNomeFicheiro() const;

    // Validar o nome do ficheiro
    bool validarNomeFicheiro(const std::string& nome) const;

public:
    // Construtor e Destrutor
    Application(const std::string& caminhoModelos);
    ~Application();

    // --- Método principal ---
    void executar();
};

#endif // APPLICATION_H
