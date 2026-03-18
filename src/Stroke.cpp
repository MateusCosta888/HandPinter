#include "Stroke.h"
#include <sstream>

// ============================================================
// Implementação da classe Stroke
// ============================================================

// Construtor padrão
Stroke::Stroke()
    : cor(cv::Scalar(0, 0, 0)), espessura(3), nomeCor("Preto"), borracha(false) {
}

// Construtor com parâmetros
Stroke::Stroke(const cv::Scalar& cor, int espessura, const std::string& nomeCor, bool borracha)
    : cor(cor), espessura(espessura), nomeCor(nomeCor), borracha(borracha) {
}

// Destrutor
Stroke::~Stroke() {
    pontos.clear();
}

// --- Getters ---
const std::vector<cv::Point>& Stroke::getPontos() const {
    return pontos;
}

cv::Scalar Stroke::getCor() const {
    return cor;
}

int Stroke::getEspessura() const {
    return espessura;
}

std::string Stroke::getNomeCor() const {
    return nomeCor;
}

bool Stroke::isBorracha() const {
    return borracha;
}

int Stroke::getNumPontos() const {
    return static_cast<int>(pontos.size());
}

// --- Métodos de modificação ---
void Stroke::adicionarPonto(const cv::Point& ponto) {
    pontos.push_back(ponto);
}

void Stroke::limpar() {
    pontos.clear();
}

// --- Serialização para texto ---
std::string Stroke::paraTexto() const {
    std::stringstream ss;
    ss << "STROKE" << std::endl;
    ss << "COR " << nomeCor << std::endl;
    ss << "BGR " << (int)cor[0] << " " << (int)cor[1] << " " << (int)cor[2] << std::endl;
    ss << "ESPESSURA " << espessura << std::endl;
    ss << "BORRACHA " << (borracha ? 1 : 0) << std::endl;

    for (const auto& p : pontos) {
        ss << "PONTO " << p.x << " " << p.y << std::endl;
    }

    ss << "FIM_STROKE" << std::endl;
    return ss.str();
}
