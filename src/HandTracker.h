#ifndef HANDTRACKER_H
#define HANDTRACKER_H

#include <vector>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>

// ============================================================
// HandTracker v8 — IA MediaPipe Real
//
// Pipeline FUNCIONAL verificado por teste:
// 1. palm_detection_lite.tflite → 2016 boxes (192x192)
// 2. hand_landmark_full.tflite → 21 landmarks (192x192)
//
// Output do hand_landmark_full:
//   Output 0 (1): presença da mão (score)
//   Output 1 (1): handedness
//   Output 2 (63): 21 landmarks × {x, y, z} em pixels 0-192
//   Output 3 (63): 21 world landmarks
//
// MediaPipe landmark IDs:
//  0=WRIST, 1=THUMB_CMC, 2=THUMB_MCP, 3=THUMB_IP, 4=THUMB_TIP
//  5=INDEX_MCP, 6=INDEX_PIP, 7=INDEX_DIP, 8=INDEX_TIP
//  9=MIDDLE_MCP, 10=MIDDLE_PIP, 11=MIDDLE_DIP, 12=MIDDLE_TIP
//  13=RING_MCP, 14=RING_PIP, 15=RING_DIP, 16=RING_TIP
//  17=PINKY_MCP, 18=PINKY_PIP, 19=PINKY_DIP, 20=PINKY_TIP
// ============================================================
class HandTracker {
private:
    cv::dnn::Net palmDetector;
    cv::dnn::Net handLandmarker;
    bool palmOK, landmarkOK;

    // Anchors para palm detection
    std::vector<float> anchorsCx, anchorsCy, anchorsW, anchorsH;

    // Estado
    bool maoDetectada;
    bool pintando;
    bool apagando;

    // Borracha Geométrica
    cv::Point centroBorracha;
    int raioBorracha;

    // 21 landmarks
    std::vector<cv::Point> landmarks;
    std::vector<cv::Point> landmarksSuaves;

    // ROI da mão
    cv::Rect handROI;
    bool temROI;

    // Focus
    cv::Point ultimoCentro;
    int framesSemMao;

    // Contorno (para compatibilidade visual)
    std::vector<cv::Point> contornoMao;

    // Dados
    int numDedos;
    int larguraFrame, alturaFrame, contadorFrames;
    std::string caminhoModelos;

    // Privados
    void gerarAnchors();
    cv::Rect detetarPalma(const cv::Mat& frame);
    bool extrairLandmarks(const cv::Mat& frame, const cv::Rect& roi);
    void suavizarPonto(cv::Point& s, const cv::Point& n, float a) const;

    // Fallback HSV
    cv::Rect detetarPeleFallback(const cv::Mat& frame);

public:
    HandTracker(const std::string& caminhoModelos);
    ~HandTracker();

    bool isMaoDetectada() const;
    bool isModelosCarregados() const;
    cv::Point getPontaIndicador() const;
    cv::Point getPontaPolegar() const;
    cv::Point getCentroMao() const;
    int getNumDedos() const;
    const std::vector<cv::Point>& getLandmarks() const;

    void processar(const cv::Mat& frame);
    bool isPintando() const;
    bool isApagando() const;
    cv::Point getCentroBorracha() const;
    int getRaioBorracha() const;
    
    void desenharLandmarks(cv::Mat& frame) const;
};

#endif
