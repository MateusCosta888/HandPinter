#ifndef MENU_H
#define MENU_H

#include <string>
#include <vector>
#include <opencv2/core.hpp>

// ============================================================
// Classe Menu
// Gere o menu inicial da aplicação:
// - Começar a Pintar
// - Configurações (escolher câmara)
// - Sair
// ============================================================
class Menu {
    int cameraId;              // ID do dispositivo de câmara selecionada
    bool deveIniciar;          // Se o utilizador escolheu "Começar"
    bool deveSair;             // Se o utilizador escolheu "Sair"

    std::vector<std::string> nomesCamaras; // Nomes amigáveis das câmaras no Registry

    // Procurar iterativamente câmaras válidas usando um ecrã de carregamento
    void procurarCamaras(cv::Mat& frame, const std::string& nomeJanela);

    // Desenhar o menu principal na janela OpenCV
    void desenharMenuPrincipal(cv::Mat& frame) const;

    // Desenhar o menu de configurações
    void desenharMenuConfiguracoes(cv::Mat& frame) const;

    // Desenhar o menu de como jogar (tutorial)
    void desenharMenuComoJogar(cv::Mat& frame) const;

public:
    // Construtor e Destrutor
    Menu();
    ~Menu();

    // --- Getters ---
    int getCameraId() const;
    bool getDeveIniciar() const;
    bool getDeveSair() const;

    // --- Setters ---
    void setCameraId(int id);

    // --- Lógica do Menu ---
    // Executar o loop do menu (retorna quando o utilizador escolhe uma opção)
    void executar();

    // Menu de configurações
    void abrirConfiguracoes();

    // Menu Como Jogar
    void abrirComoJogar();

    // Resetar o estado do menu
    void resetar();
};

#endif // MENU_H
