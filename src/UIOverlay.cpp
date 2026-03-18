#include "UIOverlay.h"
#include <opencv2/imgproc.hpp>

// ============================================================
// Implementação da classe UIOverlay
// ============================================================

UIOverlay::UIOverlay(int alturaEcra, const std::map<std::string, cv::Scalar>& paleta)
    : indiceSelecionado(0), larguraBarra(70) {
    configurarBotoes(alturaEcra, paleta);
}

UIOverlay::~UIOverlay() {
    botoes.clear();
}

// --- Configurar botões de cor ---
void UIOverlay::configurarBotoes(int larguraEcra, const std::map<std::string, cv::Scalar>& paleta) {
    botoes.clear();

    int tamanho = 45;
    int margem = 20;
    
    // Começa na área superior ao centro
    int totalLargura = (paleta.size() + 1) * tamanho + (paleta.size()) * margem;
    int x = (larguraEcra > 0 ? (larguraEcra - totalLargura) / 2 : 100); 
    int y = 15; // Altura no topo do ecrã

    // Adicionar botões de cor a partir da paleta
    for (const auto& par : paleta) {
        BotaoCor botao;
        botao.area = cv::Rect(x, y, tamanho, tamanho);
        botao.cor = par.second;
        botao.nome = par.first;
        botao.borracha = false;
        botoes.push_back(botao);
        x += tamanho + margem;
    }

    // Remover Botão de Borracha - A Borracha passou a ser gesticular (Mão Aberta inteira)
}

// --- Desenhar o indicador de estado ---
void UIOverlay::desenharIndicadorEstado(cv::Mat& frame, bool maoDetectada) const {
    int raio = 12;
    cv::Point centro(30, 35); // Canto superior esquerdo

    if (maoDetectada) {
        // Bolinha verde brilhante (com contorno escuro)
        cv::circle(frame, centro, raio+2, cv::Scalar(0, 50, 0), -1, cv::LINE_AA);
        cv::circle(frame, centro, raio, cv::Scalar(0, 255, 0), -1, cv::LINE_AA);
        cv::putText(frame, "TRACKING ON", cv::Point(55, 40),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 1, cv::LINE_AA);
    } else {
        // Bolinha vermelha
        cv::circle(frame, centro, raio+2, cv::Scalar(50, 0, 0), -1, cv::LINE_AA);
        cv::circle(frame, centro, raio, cv::Scalar(0, 0, 255), -1, cv::LINE_AA);
        cv::putText(frame, "NO HAND", cv::Point(55, 40),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
    }
}

// --- Desenhar a barra de seleção de cores ---
void UIOverlay::desenharBarraCores(cv::Mat& frame) const {
    // Top Bar Background (Horizontal Translucent Overlay)
    int hBarra = 80;
    cv::Mat barraBg = frame(cv::Rect(0, 0, frame.cols, hBarra)).clone();
    barraBg = barraBg * 0.3; // Escurecer para destaque
    barraBg.copyTo(frame(cv::Rect(0, 0, frame.cols, hBarra)));
    
    // Linha de limite
    cv::line(frame, cv::Point(0, hBarra), cv::Point(frame.cols, hBarra), cv::Scalar(255,255,255,100), 1, cv::LINE_AA);

    for (size_t i = 0; i < botoes.size(); i++) {
        const BotaoCor& botao = botoes[i];

        bool selecionado = ((int)i == indiceSelecionado);
        
        // Efeito de sombra base/Fundo
        cv::Rect sombra = botao.area;
        sombra.x += 2; sombra.y += 2;
        cv::rectangle(frame, sombra, cv::Scalar(30, 30, 30), -1, cv::LINE_AA);

        // A propriedade .borracha nos botões já não será verdadeira,
        // mas mantemos a logica do quadrado de cor por defeito
        // Desenhar quadrado de cor com contorno negro subtil para contraste
        cv::rectangle(frame, botao.area, cv::Scalar(0,0,0), -1, cv::LINE_AA);
        cv::Rect corInterior = botao.area;
        corInterior.x += 1; corInterior.y += 1; 
        corInterior.width -= 2; corInterior.height -= 2;
        cv::rectangle(frame, corInterior, botao.cor, -1, cv::LINE_AA);

        // Borda de seleção HIGHLIGHT BRILHANTE
        if (selecionado) {
            // Borda Externa Grossa
            cv::Rect glow = botao.area;
            glow.x -= 4; glow.y -= 4; glow.width += 8; glow.height += 8;
            cv::rectangle(frame, glow, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
            // Texto do nome da cor selecionada em baixo
            cv::putText(frame, botao.nome, cv::Point(botao.area.x, botao.area.y + botao.area.height + 18),
                        cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255,255,255), 1, cv::LINE_AA);
        } else {
            // Contorno subtil inativo
            cv::rectangle(frame, botao.area, cv::Scalar(180, 180, 180), 1, cv::LINE_AA);
        }
    }
}

// --- Desenhar cursor na posição do dedo ---
void UIOverlay::desenharCursor(cv::Mat& frame, const cv::Point& posicao, const cv::Scalar& cor) const {
    // Cursor mais visível misturando com sombra
    cv::circle(frame, posicao, 12, cv::Scalar(0,0,0), 3, cv::LINE_AA);
    cv::circle(frame, posicao, 10, cor, 2, cv::LINE_AA);
    cv::circle(frame, posicao, 4, cor, -1, cv::LINE_AA);
}

// --- Verificar clique em botão ---
int UIOverlay::verificarCliqueBotao(const cv::Point& posicaoDedo) const {
    for (size_t i = 0; i < botoes.size(); i++) {
        // Area de clique ligeiramente maior para ser mais tolerante (Fat finger)
        cv::Rect areaClique = botoes[i].area;
        areaClique.x -= 10; areaClique.y -= 10;
        areaClique.width += 20; areaClique.height += 20;
        
        if (areaClique.contains(posicaoDedo)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// --- Selecionar botão ---
void UIOverlay::selecionarBotao(int indice) {
    if (indice >= 0 && indice < (int)botoes.size()) {
        indiceSelecionado = indice;
    }
}

// --- Getters ---
cv::Scalar UIOverlay::getCorSelecionada() const {
    if (indiceSelecionado >= 0 && indiceSelecionado < (int)botoes.size()) {
        return botoes[indiceSelecionado].cor;
    }
    return cv::Scalar(255, 255, 255);
}

std::string UIOverlay::getNomeCorSelecionada() const {
    if (indiceSelecionado >= 0 && indiceSelecionado < (int)botoes.size()) {
        return botoes[indiceSelecionado].nome;
    }
    return "Branco";
}

bool UIOverlay::isBorrachaSelecionada() const {
    if (indiceSelecionado >= 0 && indiceSelecionado < (int)botoes.size()) {
        return botoes[indiceSelecionado].borracha;
    }
    return false;
}

int UIOverlay::getLarguraBarra() const {
    return 80; // Agora é 'Altura da barra no topo', compatibilidade nominal
}
