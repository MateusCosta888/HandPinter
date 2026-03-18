#ifndef UIOVERLAY_H
#define UIOVERLAY_H

#include <vector>
#include <string>
#include <map>
#include <opencv2/core.hpp>

// ============================================================
// Classe UIOverlay (Interface sobre a câmara)
// Desenha elementos de interface sobre a imagem da câmara:
// - Indicador de estado do rastreio (bolinha verde/vermelha)
// - Seletor de cores (5 cores + borracha)
// - Cursor do dedo
// ============================================================
class UIOverlay {
private:
    // Estrutura para um botão de cor na barra lateral
    struct BotaoCor {
        cv::Rect area;          // Área clicável
        cv::Scalar cor;         // Cor BGR
        std::string nome;       // Nome da cor
        bool borracha;          // Se é botão de borracha
    };

    std::vector<BotaoCor> botoes;      // Lista de botões
    int indiceSelecionado;              // Qual botão está selecionado
    int larguraBarra;                   // Largura da barra lateral

    // Configurar os botões com base nas dimensões do ecrã
    void configurarBotoes(int alturaEcra, const std::map<std::string, cv::Scalar>& paleta);

public:
    // Construtor
    UIOverlay(int alturaEcra, const std::map<std::string, cv::Scalar>& paleta);
    ~UIOverlay();

    // --- Renderização ---
    // Desenhar o indicador de estado (verde = mão detetada, vermelho = não)
    void desenharIndicadorEstado(cv::Mat& frame, bool maoDetectada) const;

    // Desenhar a barra de seleção de cores
    void desenharBarraCores(cv::Mat& frame) const;

    // Desenhar o cursor (circulo) na posição do dedo
    void desenharCursor(cv::Mat& frame, const cv::Point& posicao, const cv::Scalar& cor) const;

    // --- Interação ---
    // Verificar se o dedo está sobre algum botão de cor
    // Retorna o índice do botão (-1 se nenhum)
    int verificarCliqueBotao(const cv::Point& posicaoDedo) const;

    // Selecionar um botão por índice
    void selecionarBotao(int indice);

    // --- Getters ---
    cv::Scalar getCorSelecionada() const;
    std::string getNomeCorSelecionada() const;
    bool isBorrachaSelecionada() const;
    int getLarguraBarra() const;
};

#endif // UIOVERLAY_H
