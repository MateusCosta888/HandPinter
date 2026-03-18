#ifndef STROKE_H
#define STROKE_H

#include <vector>
#include <string>
#include <opencv2/core.hpp>

// ============================================================
// Classe Stroke (Traço)
// Representa um traço contínuo feito pelo utilizador.
// Guarda uma sequência de pontos, a cor e a espessura.
// ============================================================
class Stroke {
private:
    std::vector<cv::Point> pontos;     // Coordenadas do traço
    cv::Scalar cor;                     // Cor do traço (BGR)
    int espessura;                      // Espessura da linha
    std::string nomeCor;                // Nome da cor (ex: "Vermelho")
    bool borracha;                      // Se este traço é uma borracha

public:
    // Construtores
    Stroke();
    Stroke(const cv::Scalar& cor, int espessura, const std::string& nomeCor, bool borracha = false);

    // Destrutor
    ~Stroke();

    // --- Métodos de acesso (Getters) ---
    const std::vector<cv::Point>& getPontos() const;
    cv::Scalar getCor() const;
    int getEspessura() const;
    std::string getNomeCor() const;
    bool isBorracha() const;
    int getNumPontos() const;

    // --- Métodos de modificação ---
    void adicionarPonto(const cv::Point& ponto);
    void limpar();

    // --- Serialização (ficheiro) ---
    std::string paraTexto() const;
};

#endif // STROKE_H
