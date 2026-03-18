#include "HandTracker.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <opencv2/imgproc.hpp>

// ============================================================
// HandTracker v8 — IA MediaPipe Real
//
// Modelos VERIFICADOS funcionais via teste de diagnóstico:
// - palm_detection_lite.tflite: 2016 detections, input 192x192
// - hand_landmark_full.tflite: 21 landmarks, input 192x192
//
// Pipeline:
// 1. Palm detection → encontra bounding box da mão
// 2. Crop + resize → alimentar ao landmark model
// 3. 21 landmarks com coordenadas reais de cada dedo
// 4. Pinça = distância entre landmark 4 (polegar) e 8 (indicador)
// ============================================================

static const int PALM_INPUT = 192;
static const int LANDMARK_INPUT = 224;  // Rede FULL devolve landmarks na escala 224x224
static const int NUM_ANCHORS = 2016;
static const int NUM_COORD = 18;
static const float PALM_THRESHOLD = 0.5f;

HandTracker::HandTracker(const std::string& caminho)
    : palmOK(false), landmarkOK(false),
      maoDetectada(false), pintando(false), apagando(false),
      centroBorracha(0,0), raioBorracha(0),
      temROI(false), ultimoCentro(-1, -1), framesSemMao(0),
      numDedos(0), larguraFrame(0), alturaFrame(0), contadorFrames(0),
      caminhoModelos(caminho) {

    landmarks.resize(21, cv::Point(-9999, -9999));
    landmarksSuaves.resize(21, cv::Point(-9999, -9999));

    // Carregar palm detection
    try {
        palmDetector = cv::dnn::readNetFromTFLite(caminho + "/palm_detection_lite.tflite");
        if (!palmDetector.empty()) { palmOK = true; std::cout << "[OK] Palm detection carregado" << std::endl; }
    } catch (const cv::Exception& e) {
        std::cerr << "[ERRO] Palm detection: " << e.what() << std::endl;
    }

    // Carregar hand landmark (FULL, não lite!)
    try {
        handLandmarker = cv::dnn::readNetFromTFLite(caminho + "/hand_landmark_full.tflite");
        if (!handLandmarker.empty()) { landmarkOK = true; std::cout << "[OK] Hand landmark FULL carregado" << std::endl; }
    } catch (const cv::Exception& e) {
        std::cerr << "[ERRO] Hand landmark: " << e.what() << std::endl;
    }

    // Gerar anchors
    gerarAnchors();

    std::cout << "HandTracker v8 (IA MediaPipe Real)" << std::endl;
    if (palmOK && landmarkOK)
        std::cout << "  Modo: IA completa (palm + landmark)" << std::endl;
    else if (landmarkOK)
        std::cout << "  Modo: Landmark IA + HSV palm" << std::endl;
    else
        std::cout << "  Modo: Fallback HSV apenas" << std::endl;
}

HandTracker::~HandTracker() {}

// ============================================================
// Gerar anchors programaticamente (igual ao MediaPipe)
// ============================================================
void HandTracker::gerarAnchors() {
    struct LayerInfo { int stride; int numAnchors; };
    LayerInfo layers[] = { {8, 2}, {16, 6} };

    anchorsCx.clear(); anchorsCy.clear();
    anchorsW.clear(); anchorsH.clear();

    for (auto& layer : layers) {
        int gridH = PALM_INPUT / layer.stride;
        int gridW = PALM_INPUT / layer.stride;
        for (int y = 0; y < gridH; y++) {
            for (int x = 0; x < gridW; x++) {
                for (int a = 0; a < layer.numAnchors; a++) {
                    float cx = (x + 0.5f) / gridW;
                    float cy = (y + 0.5f) / gridH;
                    anchorsCx.push_back(cx);
                    anchorsCy.push_back(cy);
                    anchorsW.push_back(1.0f);
                    anchorsH.push_back(1.0f);
                }
            }
        }
    }
    std::cout << "  Anchors geradas: " << anchorsCx.size() << std::endl;
}

// --- Getters ---
bool HandTracker::isMaoDetectada() const { return maoDetectada; }
bool HandTracker::isModelosCarregados() const { return palmOK && landmarkOK; }

cv::Point HandTracker::getPontaIndicador() const {
    return (maoDetectada && landmarksSuaves[8].x != -9999) ? landmarksSuaves[8] : cv::Point(-1,-1);
}
cv::Point HandTracker::getPontaPolegar() const {
    return (maoDetectada && landmarksSuaves[4].x != -9999) ? landmarksSuaves[4] : cv::Point(-1,-1);
}
cv::Point HandTracker::getCentroMao() const {
    return (maoDetectada && landmarksSuaves[0].x != -9999) ? landmarksSuaves[0] : cv::Point(-1,-1);
}
int HandTracker::getNumDedos() const { return numDedos; }
const std::vector<cv::Point>& HandTracker::getLandmarks() const { return landmarksSuaves; }

// --- Suavização ---
void HandTracker::suavizarPonto(cv::Point& s, const cv::Point& n, float a) const {
    if (s.x == -9999 || s.y == -9999) s = n;
    else {
        s.x = (int)(a * s.x + (1.0f - a) * n.x);
        s.y = (int)(a * s.y + (1.0f - a) * n.y);
    }
}

// ============================================================
// PALM DETECTION
// Input: 192x192, normalizado [-1, 1]
// Output 0: scores [1, 2016, 1]
// Output 1: boxes [1, 2016, 18]
// ============================================================
cv::Rect HandTracker::detetarPalma(const cv::Mat& frame) {
    if (!palmOK) return detetarPeleFallback(frame);

    try {
        cv::Mat input;
        cv::resize(frame, input, cv::Size(PALM_INPUT, PALM_INPUT));
        cv::cvtColor(input, input, cv::COLOR_BGR2RGB);
        input.convertTo(input, CV_32F, 1.0 / 127.5, -1.0);

        cv::Mat blob = cv::dnn::blobFromImage(input);
        palmDetector.setInput(blob);

        std::vector<cv::String> outNames = palmDetector.getUnconnectedOutLayersNames();
        std::vector<cv::Mat> outputs;
        palmDetector.forward(outputs, outNames);

        // Encontrar qual output é scores (2016x1) e qual é boxes (2016x18)
        float* scoreData = nullptr;
        float* boxData = nullptr;
        for (auto& out : outputs) {
            int total = (int)out.total();
            if (total == NUM_ANCHORS) scoreData = (float*)out.data;
            else if (total == NUM_ANCHORS * NUM_COORD) boxData = (float*)out.data;
        }

        if (!scoreData || !boxData) return detetarPeleFallback(frame);

        // Encontrar a melhor deteção
        float bestScore = PALM_THRESHOLD;
        int bestIdx = -1;

        for (int i = 0; i < NUM_ANCHORS; i++) {
            float score = 1.0f / (1.0f + exp(-scoreData[i]));  // Sigmoid
            if (score > bestScore) {
                bestScore = score;
                bestIdx = i;
            }
        }

        if (bestIdx < 0) return cv::Rect();

        // Decodificar box
        int off = bestIdx * NUM_COORD;
        float cx = boxData[off + 0] / PALM_INPUT + anchorsCx[bestIdx];
        float cy = boxData[off + 1] / PALM_INPUT + anchorsCy[bestIdx];
        float w  = boxData[off + 2] / PALM_INPUT;
        float h  = boxData[off + 3] / PALM_INPUT;

        // A Palm Detection deteta APENAS a palma central.
        // Guidelines do MediaPipe:
        // 1. Transladar o centro (cy) para cima (em direção aos dedos). -0.5 * h.
        // 2. Expandir a caixa significativamente para encapsular os dedos compridos (scale = 2.6).
        float shift_y = -0.5f;
        float scale_box = 2.6f;

        cy += shift_y * h;
        w *= scale_box;
        h *= scale_box;

        // Converter para coordenadas do frame
        int px = (int)((cx - w/2) * larguraFrame);
        int py = (int)((cy - h/2) * alturaFrame);
        int pw = (int)(w * larguraFrame);
        int ph = (int)(h * alturaFrame);

        // Tornar quadrado
        int lado = std::max(pw, ph);
        int ccx = px + pw/2, ccy = py + ph/2;
        px = std::max(0, ccx - lado/2);
        py = std::max(0, ccy - lado/2);
        pw = std::min(larguraFrame - px, lado);
        ph = std::min(alturaFrame - py, lado);

        return cv::Rect(px, py, pw, ph);

    } catch (const cv::Exception& e) {
        return detetarPeleFallback(frame);
    }
}

// ============================================================
// HAND LANDMARKS
// Input: 224x224 (crop da mão), normalizado [0, 1]
// Output: 21 landmarks × {x, y, z} em pixels 0-224
// ============================================================
bool HandTracker::extrairLandmarks(const cv::Mat& frame, const cv::Rect& roi) {
    if (!landmarkOK || roi.empty() || roi.width < 20 || roi.height < 20) return false;

    try {
        // Criar um ROI Ideal em Quadrado Perfeito, usando o ratio da deteção da palma
        int lado = std::max(roi.width, roi.height);
        int cx = roi.x + roi.width / 2;
        int cy = roi.y + roi.height / 2;
        
        // Coordenadas ideais (estas podem e DEVEM vazar do frame intencionalmente!)
        int idealX = cx - lado / 2;
        int idealY = cy - lado / 2;
        cv::Rect idealROI(idealX, idealY, lado, lado);
        
        // Calcular a intersecao real com a captura de OpenCV (Janela física)
        cv::Rect frameRect(0, 0, frame.cols, frame.rows);
        cv::Rect validROI = idealROI & frameRect;
        
        if (validROI.width < 20 || validROI.height < 20) return false;

        // Montar a imagem de IA mantendo o aspect ratio de 1:1 rigoroso, preenchendo o vazio da tela física transponível com pixeis Pretos nulos.
        cv::Mat crop = cv::Mat::zeros(lado, lado, CV_8UC3);
        int dx = std::max(0, -idealX); // offset base X de preenchimento
        int dy = std::max(0, -idealY); // offset base Y de preenchimento
        frame(validROI).copyTo(crop(cv::Rect(dx, dy, validROI.width, validROI.height)));

        cv::Mat input;
        cv::resize(crop, input, cv::Size(LANDMARK_INPUT, LANDMARK_INPUT));
        cv::cvtColor(input, input, cv::COLOR_BGR2RGB);
        input.convertTo(input, CV_32F, 1.0 / 255.0);

        cv::Mat blob = cv::dnn::blobFromImage(input);
        handLandmarker.setInput(blob);

        std::vector<cv::String> outNames = handLandmarker.getUnconnectedOutLayersNames();
        std::vector<cv::Mat> outputs;
        handLandmarker.forward(outputs, outNames);

        // Encontrar os outputs das sub-camadas Multi-Layer CNN
        float* landmarkData = nullptr;
        float presenceScore = 0;
        bool scoreEncontrado = false;

        for (auto& out : outputs) {
            int total = (int)out.total();
            float* data = (float*)out.data;

            if (total == 63 && !landmarkData) {
                // Primeiro output de 63 = posicoes X,Y,Z em array (escala local de 224)
                landmarkData = data;
            } else if (total == 1 && !scoreEncontrado) {
                // MediaPipe outputs: 1º[Presence Score], 2º[Handedness].
                // Lemos apenas o primeiro size=1 para impedir a fuga de probabilidade cruzada!
                presenceScore = 1.0f / (1.0f + exp(-data[0]));
                scoreEncontrado = true;
            }
        }

        // Exige confiança superior a 50% para aceitar que o recorte contém pele humana vs ruído HSV!
        if (!landmarkData || presenceScore < 0.5f) return false;

        // Extrair pontos chave: Coordenadas da rede Neuronal MediaPipe
        for (int i = 0; i < 21; i++) {
            float lx = landmarkData[i * 3 + 0];
            float ly = landmarkData[i * 3 + 1];

            // Reconverter coordenadas 224x224 para o Quadrado de Corte Virtual
            int pX_local = (int)((lx / (float)LANDMARK_INPUT) * lado);
            int pY_local = (int)((ly / (float)LANDMARK_INPUT) * lado);

            // A Matriz Preta (Crop) começa rigorosamente em idealX, idealY (que pode ser negativo)
            // Os pixeis devolvidos mapeiam linearmente do canto esquerdo desse quadrado virtual
            int px = idealX + pX_local;
            int py = idealY + pY_local;
            
            // Removido o clamping: a rede pode e DEVE prever pontos que escapem à margem visível do ecrã!
            landmarks[i] = cv::Point(px, py);
        }

        return true;

    } catch (const cv::Exception& e) {
        return false;
    }
}

// --- Fallback HSV ---
cv::Rect HandTracker::detetarPeleFallback(const cv::Mat& frame) {
    cv::Mat hsv, m1, m2, mask;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
    cv::inRange(hsv, cv::Scalar(0,30,60), cv::Scalar(20,180,255), m1);
    cv::inRange(hsv, cv::Scalar(160,30,60), cv::Scalar(180,180,255), m2);
    mask = m1 | m2;
    cv::Mat k = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(9,9));
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, k);
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, k);

    std::vector<std::vector<cv::Point>> cs;
    cv::findContours(mask, cs, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    double maxA = 0; int maxI = -1;
    for (size_t i = 0; i < cs.size(); i++) {
        double a = cv::contourArea(cs[i]);
        if (a > maxA && a > 4000) { maxA = a; maxI = (int)i; }
    }
    if (maxI < 0) return cv::Rect();

    cv::Rect bbox = cv::boundingRect(cs[maxI]);
    int pad = (int)(std::max(bbox.width, bbox.height) * 0.3);
    bbox.x = std::max(0, bbox.x-pad); bbox.y = std::max(0, bbox.y-pad);
    bbox.width = std::min(frame.cols - bbox.x, bbox.width+2*pad);
    bbox.height = std::min(frame.rows - bbox.y, bbox.height+2*pad);

    // Tornar quadrado
    int lado = std::max(bbox.width, bbox.height);
    int cx = bbox.x + bbox.width/2, cy = bbox.y + bbox.height/2;
    bbox.x = std::max(0, cx-lado/2); bbox.y = std::max(0, cy-lado/2);
    bbox.width = std::min(frame.cols - bbox.x, lado);
    bbox.height = std::min(frame.rows - bbox.y, lado);
    return bbox;
}

// ============================================================
// PROCESSAR FRAME
// Estratégia: HSV para encontrar a mão + IA para landmarks
// ============================================================
void HandTracker::processar(const cv::Mat& frame) {
    larguraFrame = frame.cols;
    alturaFrame = frame.rows;
    contadorFrames++;

    // 1. Deteção da Mão: IA Palm Detection (Ignora Braços e Rostos)
    cv::Rect roi;

    // A MÁGICA DO TRACKING ESTÁVEL:
    // A MÁGICA DO TRACKING ESTÁVEL E RÁPIDO:
    // Expansão agressiva (80%) da bounding box dos landmarks para o próximo frame
    // Isto garante que movimentos ultrarrápidos da mão não escapem à ROIC de tracking!
    if (temROI && handROI.width > 0) {
        int padX = (int)(handROI.width * 0.8);
        int padY = (int)(handROI.height * 0.8);
        
        // CORRECÇÃO GEOMÉTRICA DE EXPLOSÃO:
        // O ROI precisa de ser novamente "Clampado" contra as dimensões físicas do Frame do OpenCV para evitar Feedback de Crescimento Infinito.
        // Já não podemos deixar que 'roi' fuja do ecrã, porque o Extractor 'extrairLandmarks' constrói um ROI Ideal (Zero-padded) baseado na imagem enviada! 
        // Se a caixa real C++ passar do ecrã, a Box aumenta +80% a cada Frame sem limites, congelando o CPU.
        roi.x = std::max(0, handROI.x - padX);
        roi.y = std::max(0, handROI.y - padY);
        roi.width = std::min(larguraFrame - roi.x, handROI.width + 2*padX);
        roi.height = std::min(alturaFrame - roi.y, handROI.height + 2*padY);
    } else {
        // Pesquisa global de Palmas apenas quando o tracking foi totalmente perdido
        roi = detetarPalma(frame);
    }

    // Fallback absoluto: Se a rede não encontrou palma, tenta o HSV (usado no pior caso)
    if (roi.empty()) {
        roi = detetarPeleFallback(frame);
    }

    if (roi.empty() || roi.width < 30) {
        maoDetectada = false;
        pintando = false;
        apagando = false;
        temROI = false;
        // Limpar posições antigas do esqueleto à força para remover "fantasmas"
        for (int i = 0; i < 21; i++) {
            landmarks[i] = cv::Point(-9999, -9999);
            landmarksSuaves[i] = cv::Point(-9999, -9999);
        }
        return;
    }

    // 2. Alimentar o modelo IA com a região da mão
    bool ok = extrairLandmarks(frame, roi);

    if (!ok) {
        framesSemMao++;
        if (framesSemMao > 3) {
            maoDetectada = false;
            pintando = false;
            apagando = false;
            temROI = false;
            // Forçar limpeza em falha da IA (Esqueleto Fantasma Fix)
            for (int i = 0; i < 21; i++) {
                landmarks[i] = cv::Point(-9999, -9999);
                landmarksSuaves[i] = cv::Point(-9999, -9999);
            }
        }
        return;
    }

    // 3. Suavizar TODOS os 21 landmarks (Exponential Moving Average)
    // Se estavamos "Sem Mão", os landmarks suaves não devem ser arrastados a partir do zero
    if (framesSemMao > 0) {
        for (int i = 0; i < 21; i++) {
            landmarksSuaves[i] = landmarks[i]; // Saltar direto
        }
    } else {
        // Inércia menor (0.3) para ser SUPER RÁPIDO e não se arrastar atrás da mão
        for (int i = 0; i < 21; i++) {
            suavizarPonto(landmarksSuaves[i], landmarks[i], 0.3f);
        }
    }

    framesSemMao = 0;

    // 4. Contar dedos
    numDedos = 0;
    double tDist = cv::norm(landmarksSuaves[4] - landmarksSuaves[2]);
    double tBase = cv::norm(landmarksSuaves[3] - landmarksSuaves[2]);
    if (tDist > tBase * 0.8) numDedos++;
    if (landmarksSuaves[8].y < landmarksSuaves[6].y) numDedos++;
    if (landmarksSuaves[12].y < landmarksSuaves[10].y) numDedos++;
    if (landmarksSuaves[16].y < landmarksSuaves[14].y) numDedos++;
    if (landmarksSuaves[20].y < landmarksSuaves[18].y) numDedos++;


    // 5. Detetar GESTO DE PINTURA (Apenas Indicador Levantado)
    // O indicador (Tip 8) tem de estar ACIMA da sua base (PIP 6).
    // Os outros dedos (Médio 12, Anelar 16, Mindinho 20) devem estar ABAIXO das suas bases (PIP 10, 14, 18).
    // (Lembrar que a coordenada Y=0 é no topo do ecrã, logo "ACIMA" significa menor Y).
    bool indUp    = (landmarksSuaves[8].y < landmarksSuaves[6].y);
    bool medDown  = (landmarksSuaves[12].y > landmarksSuaves[10].y);
    bool ringDown = (landmarksSuaves[16].y > landmarksSuaves[14].y);
    bool pinkyDown = (landmarksSuaves[20].y > landmarksSuaves[18].y);

    pintando = (indUp && medDown && ringDown && pinkyDown);
    
    // Gesto de Borracha Dinâmica: Punho fechado (todos os 4 dedos principais recolhidos)
    // Ignoramos o polegar pois o cálculo de norm() produz falsos positivos com a mão rodada.
    apagando = (!indUp && medDown && ringDown && pinkyDown);
    if (apagando) {
        centroBorracha = landmarksSuaves[9]; // Nó do dedo médio = bom centro na palma
        raioBorracha = cv::norm(landmarksSuaves[0] - landmarksSuaves[9]) * 0.5; // Escala 50% menor para caber no punho
    } else {
        raioBorracha = 0;
    }

    // 6. Atualizar estado
    maoDetectada = true;
    ultimoCentro = landmarksSuaves[0];

    // Calcular ROI a partir dos landmarks (MÁXIMO LIMITE PARA O PRÓXIMO FRAME)
    int minX = larguraFrame, maxX = 0, minY = alturaFrame, maxY = 0;
    for (int i = 0; i < 21; i++) {
        if (landmarksSuaves[i].x != -9999) {
            // Rebitamos o min/max contra os limites do Ecrã para que as previsões virtuais livres ('OffScreen') 
            // não destruam as dimensões limpas da caixa de Crop seguinte!
            int tX = std::max(0, std::min(larguraFrame - 1, landmarksSuaves[i].x));
            int tY = std::max(0, std::min(alturaFrame - 1, landmarksSuaves[i].y));

            minX = std::min(minX, tX);
            maxX = std::max(maxX, tX);
            minY = std::min(minY, tY);
            maxY = std::max(maxY, tY);
        }
    }
    if (maxX > minX && maxY > minY && (maxX - minX) > 20) {
        handROI = cv::Rect(minX, minY, maxX-minX, maxY-minY);
        temROI = true;
    } else {
        temROI = false; // Se a caixa recolhida for minuscula/falsa, perdemos tracking visual.
    }
}

// --- A pintura está ativa? ---
bool HandTracker::isPintando() const {
    return maoDetectada && pintando;
}

// --- Borracha Dinâmica ---
bool HandTracker::isApagando() const {
    return maoDetectada && apagando;
}

cv::Point HandTracker::getCentroBorracha() const {
    return centroBorracha;
}

int HandTracker::getRaioBorracha() const {
    return raioBorracha;
}

// ============================================================
// DESENHAR — esqueleto completo da mão com 21 landmarks
// ============================================================
void HandTracker::desenharLandmarks(cv::Mat& frame) const {
    if (!maoDetectada) return;
    
    // Assegurar que os landmarks não são zeros (fantasmas limpados mas flag desatualizada)
    if (landmarksSuaves[0].x == -9999) return;

    cv::Scalar corPintura = pintando ? cv::Scalar(0,255,0) : cv::Scalar(0,165,255);

    // Esqueleto
    int conn[][2] = {
        {0,1},{1,2},{2,3},{3,4},         // Polegar
        {0,5},{5,6},{6,7},{7,8},         // Indicador
        {0,9},{9,10},{10,11},{11,12},     // Médio
        {0,13},{13,14},{14,15},{15,16},   // Anelar
        {0,17},{17,18},{18,19},{19,20},   // Mindinho
        {5,9},{9,13},{13,17}              // Palma
    };

    for (auto& c : conn) {
        if (landmarksSuaves[c[0]].x != -9999 && landmarksSuaves[c[1]].x != -9999) {
            cv::line(frame, landmarksSuaves[c[0]], landmarksSuaves[c[1]],
                     cv::Scalar(0,200,200), 2, cv::LINE_AA);
        }
    }

    // Pontos landmarks
    for (int i = 0; i < 21; i++) {
        if (landmarksSuaves[i].x == -9999) continue;
        cv::Scalar cor = cv::Scalar(180,180,180);
        int r = 3;
        if (i == 8) { cor = cv::Scalar(0,255,0); r = 8; }
        else if (i == 4) { cor = cv::Scalar(255,100,0); r = 7; }
        else if (i == 0) { cor = cv::Scalar(0,255,255); r = 5; }
        else if (i==12||i==16||i==20) { cor = cv::Scalar(200,200,200); r = 4; }
        cv::circle(frame, landmarksSuaves[i], r, cor, -1, cv::LINE_AA);
    }

    // Labels
    if (landmarksSuaves[8].x != -9999)
        cv::putText(frame, "Indicador", landmarksSuaves[8]+cv::Point(12,-5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0,255,0), 1, cv::LINE_AA);
    if (landmarksSuaves[4].x != -9999)
        cv::putText(frame, "Polegar", landmarksSuaves[4]+cv::Point(12,-5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255,100,0), 1, cv::LINE_AA);

    // Linha indicativa
    if (landmarksSuaves[4].x != -9999 && landmarksSuaves[8].x != -9999) {
        cv::Scalar lc = pintando ? cv::Scalar(0,255,0) : cv::Scalar(0,0,200);
        cv::line(frame, landmarksSuaves[4], landmarksSuaves[8], lc,
                 pintando ? 3 : 1, cv::LINE_AA);
    }

    // Info
    std::string estado = pintando ? "A PINTAR" : "MAO OK";
    cv::putText(frame, estado, cv::Point(frame.cols-220, 30),
                cv::FONT_HERSHEY_SIMPLEX, 0.5, corPintura, 1, cv::LINE_AA);
    cv::putText(frame, "Dedos: " + std::to_string(numDedos),
                cv::Point(frame.cols-140, 55),
                cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(255,255,255), 1, cv::LINE_AA);
    cv::putText(frame, "[IA MediaPipe]", cv::Point(frame.cols-140, 78),
                cv::FONT_HERSHEY_SIMPLEX, 0.35, cv::Scalar(100,255,100), 1, cv::LINE_AA);
}
