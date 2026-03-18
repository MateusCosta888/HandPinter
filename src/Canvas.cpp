#include "Canvas.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <opencv2/imgproc.hpp>

// ============================================================
// Implementação da classe Canvas
// ============================================================

Canvas::Canvas(int largura, int altura)
    : largura(largura), altura(altura) {
    // Criar imagem transparente (preta com canal alfa)
    imagem = cv::Mat::zeros(altura, largura, CV_8UC3);
    inicializarPaleta();
}

Canvas::~Canvas() {
    // Liberta todos os Stroke* com delete (requisito 4: apontadores)
    for (Stroke* s : tracos) {
        delete s;
    }
    tracos.clear();
}

// --- Inicializar a paleta de cores ---
void Canvas::inicializarPaleta() {
    // BGR (não RGB!)
    paleta["Vermelho"]  = cv::Scalar(0, 0, 255);
    paleta["Azul"]      = cv::Scalar(255, 100, 0);
    paleta["Verde"]     = cv::Scalar(0, 200, 0);
    paleta["Amarelo"]   = cv::Scalar(0, 255, 255);
    paleta["Branco"]    = cv::Scalar(255, 255, 255);
}

// --- Getters ---
int Canvas::getLargura() const { return largura; }
int Canvas::getAltura() const { return altura; }
int Canvas::getNumTracos() const { return static_cast<int>(tracos.size()); }

const std::map<std::string, cv::Scalar>& Canvas::getPaleta() const {
    return paleta;
}

cv::Scalar Canvas::getCorPorNome(const std::string& nome) const {
    auto it = paleta.find(nome);
    if (it != paleta.end()) {
        return it->second;
    }
    return cv::Scalar(255, 255, 255); // Branco por defeito
}

std::vector<std::string> Canvas::getNomesCores() const {
    std::vector<std::string> nomes;
    for (const auto& par : paleta) {
        nomes.push_back(par.first);
    }
    return nomes;
}

// --- Gestão de traços ---
void Canvas::iniciarNovoTraco(const cv::Scalar& cor, int espessura, const std::string& nomeCor, bool borracha) {
    // Alocar um novo Stroke com new (requisito 4: apontadores)
    Stroke* novoTraco = new Stroke(cor, espessura, nomeCor, borracha);
    tracos.push_back(novoTraco);
}

void Canvas::adicionarPontoAoTracoAtual(const cv::Point& ponto) {
    if (!tracos.empty()) {
        tracos.back()->adicionarPonto(ponto);
    }
}

void Canvas::limparTela() {
    // Liberta todos os Stroke* com delete
    for (Stroke* s : tracos) {
        delete s;
    }
    tracos.clear();
    imagem = cv::Mat::zeros(altura, largura, CV_8UC3);
}

// --- Renderização ---
cv::Mat Canvas::renderizar() const {
    cv::Mat resultado = cv::Mat::zeros(altura, largura, CV_8UC3);

    for (const Stroke* s : tracos) {
        const std::vector<cv::Point>& pontos = s->getPontos();
        if (pontos.size() < 2) continue;

        if (s->isBorracha()) {
            // Borracha: esfera que desenha a preto (apaga) em movimento
            for (size_t i = 1; i < pontos.size(); i++) {
                cv::line(resultado, pontos[i - 1], pontos[i],
                         cv::Scalar(0, 0, 0), s->getEspessura(),
                         cv::LINE_AA);
                // "Rounded Caps" - desenha circulos nas arestas para a borracha parecer uma bola sólida
                cv::circle(resultado, pontos[i], s->getEspessura() / 2, cv::Scalar(0,0,0), -1, cv::LINE_AA);
            }
            if (!pontos.empty()) {
                cv::circle(resultado, pontos[0], s->getEspessura() / 2, cv::Scalar(0,0,0), -1, cv::LINE_AA);
            }
        } else {
            // Desenhar linhas entre pontos consecutivos
            for (size_t i = 1; i < pontos.size(); i++) {
                cv::line(resultado, pontos[i - 1], pontos[i],
                         s->getCor(), s->getEspessura(),
                         cv::LINE_AA);
            }
        }
    }

    return resultado;
}

// --- Guardar em ficheiro de texto ---
bool Canvas::guardarFicheiro(const std::string& nomeFicheiro) const {
    std::ofstream ficheiro(nomeFicheiro);
    if (!ficheiro.is_open()) {
        std::cerr << "Erro: Nao foi possivel abrir o ficheiro para escrita: " << nomeFicheiro << std::endl;
        return false;
    }

    ficheiro << "# HandPaint - Ficheiro de Desenho" << std::endl;
    ficheiro << "LARGURA " << largura << std::endl;
    ficheiro << "ALTURA " << altura << std::endl;
    ficheiro << "NUM_TRACOS " << tracos.size() << std::endl;
    ficheiro << std::endl;

    for (const Stroke* s : tracos) {
        ficheiro << s->paraTexto() << std::endl;
    }

    ficheiro.close();
    std::cout << "Desenho guardado com sucesso em: " << nomeFicheiro << std::endl;
    return true;
}

// --- Carregar de ficheiro de texto ---
bool Canvas::carregarFicheiro(const std::string& nomeFicheiro) {
    std::ifstream ficheiro(nomeFicheiro);
    if (!ficheiro.is_open()) {
        std::cerr << "Erro: Nao foi possivel abrir o ficheiro: " << nomeFicheiro << std::endl;
        return false;
    }

    // Limpar tela atual
    limparTela();

    std::string linha;
    Stroke* tracoAtual = nullptr;

    while (std::getline(ficheiro, linha)) {
        // Ignorar comentários e linhas vazias
        if (linha.empty() || linha[0] == '#') continue;

        std::istringstream iss(linha);
        std::string comando;
        iss >> comando;

        if (comando == "LARGURA") {
            iss >> largura;
        } else if (comando == "ALTURA") {
            iss >> altura;
        } else if (comando == "STROKE") {
            // Criar novo traço (será preenchido nas linhas seguintes)
            tracoAtual = new Stroke();
            tracos.push_back(tracoAtual);
        } else if (comando == "COR" && tracoAtual) {
            // Ignorar (será reconstruído a partir do BGR)
        } else if (comando == "BGR" && tracoAtual) {
            int b, g, r;
            iss >> b >> g >> r;
            // Reconstruir o Stroke com a cor lida
            // (é preciso guardar temporariamente)
        } else if (comando == "PONTO" && tracoAtual) {
            int x, y;
            iss >> x >> y;
            tracoAtual->adicionarPonto(cv::Point(x, y));
        } else if (comando == "FIM_STROKE") {
            tracoAtual = nullptr;
        }
    }

    ficheiro.close();
    std::cout << "Desenho carregado com sucesso de: " << nomeFicheiro << std::endl;
    return true;
}
