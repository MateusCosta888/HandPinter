#ifndef CANVAS_H
#define CANVAS_H

#include <vector>
#include <string>
#include <map>
#include <opencv2/core.hpp>

#include "Stroke.h"

// ============================================================
// Classe Canvas (Tela de Desenho)
// Gere a coleção de traços (Strokes) desenhados pelo utilizador.
// Suporta guardar/carregar de ficheiro de texto.
// Relação: Canvas contém Stroke* (Agregação com apontadores)
// ============================================================
class Canvas {
private:
    std::vector<Stroke*> tracos;       // Apontadores para os traços (new/delete)
    int largura;                        // Largura da tela
    int altura;                         // Altura da tela
    cv::Mat imagem;                     // Imagem da tela (para desenho)

    // Paleta de cores disponíveis (std::map)
    std::map<std::string, cv::Scalar> paleta;

    // Inicializar a paleta de cores padrão
    void inicializarPaleta();

public:
    // Construtor e Destrutor
    Canvas(int largura, int altura);
    ~Canvas();  // Liberta todos os Stroke* com delete!

    // --- Getters ---
    int getLargura() const;
    int getAltura() const;
    int getNumTracos() const;
    const std::map<std::string, cv::Scalar>& getPaleta() const;
    cv::Scalar getCorPorNome(const std::string& nome) const;
    std::vector<std::string> getNomesCores() const;

    // --- Gestão de traços ---
    void iniciarNovoTraco(const cv::Scalar& cor, int espessura, const std::string& nomeCor, bool borracha = false);
    void adicionarPontoAoTracoAtual(const cv::Point& ponto);
    void limparTela();  // Liberta todos os Stroke* e limpa a imagem

    // --- Renderização ---
    cv::Mat renderizar() const;

    // --- Persistência em ficheiro de texto ---
    bool guardarFicheiro(const std::string& nomeFicheiro) const;
    bool carregarFicheiro(const std::string& nomeFicheiro);
};

#endif // CANVAS_H
