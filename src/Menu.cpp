#include "Menu.h"
#include <iostream>
#include <string>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <windows.h>
#include <dshow.h>

// ============================================================
// Implementação da classe Menu
// ============================================================

Menu::Menu()
    : cameraId(0), deveIniciar(false), deveSair(false) {
}

Menu::~Menu() {
}

// --- Getters ---
int Menu::getCameraId() const { return cameraId; }
bool Menu::getDeveIniciar() const { return deveIniciar; }
bool Menu::getDeveSair() const { return deveSair; }

// --- Setters ---
void Menu::setCameraId(int id) {
    if (id >= 0 && id <= 10) {
        cameraId = id;
    }
}

// --- Desenhar o menu principal ---
void Menu::desenharMenuPrincipal(cv::Mat& frame) const {
    // Fundo escuro gradiente
    frame = cv::Scalar(30, 30, 40);

    int largura = frame.cols;
    int altura = frame.rows;

    // Título
    std::string titulo = "HandPaint";
    int fontFace = cv::FONT_HERSHEY_DUPLEX;
    double fontScale = 2.0;
    int thickness = 3;
    cv::Size textSize = cv::getTextSize(titulo, fontFace, fontScale, thickness, nullptr);
    cv::Point tituloPos((largura - textSize.width) / 2, 100);
    cv::putText(frame, titulo, tituloPos, fontFace, fontScale,
                cv::Scalar(100, 220, 255), thickness, cv::LINE_AA);

    // Subtítulo
    std::string sub = "Pinta com as tuas maos!";
    cv::Size subSize = cv::getTextSize(sub, cv::FONT_HERSHEY_SIMPLEX, 0.7, 1, nullptr);
    cv::putText(frame, sub, cv::Point((largura - subSize.width) / 2, 140),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(180, 180, 180), 1, cv::LINE_AA);

    // Botões do menu
    int btnLargura = 300;
    int btnAltura = 45;
    int btnX = (largura - btnLargura) / 2;
    int btnY = 170;
    int espaco = 15;

    // Botão 1: Começar a Pintar
    cv::Rect btn1(btnX, btnY, btnLargura, btnAltura);
    cv::rectangle(frame, btn1, cv::Scalar(0, 180, 100), -1, cv::LINE_AA);
    cv::rectangle(frame, btn1, cv::Scalar(0, 220, 130), 2, cv::LINE_AA);
    std::string txt1 = "[1] Comecar a Pintar";
    cv::Size t1 = cv::getTextSize(txt1, cv::FONT_HERSHEY_SIMPLEX, 0.65, 2, nullptr);
    cv::putText(frame, txt1,
                cv::Point(btnX + (btnLargura - t1.width) / 2, btnY + (btnAltura + t1.height) / 2),
                cv::FONT_HERSHEY_SIMPLEX, 0.65, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);

    // Botão 2: Configurações
    btnY += btnAltura + espaco;
    cv::Rect btn2(btnX, btnY, btnLargura, btnAltura);
    cv::rectangle(frame, btn2, cv::Scalar(180, 120, 0), -1, cv::LINE_AA);
    cv::rectangle(frame, btn2, cv::Scalar(220, 150, 0), 2, cv::LINE_AA);
    std::string txt2 = "[2] Configuracoes";
    cv::Size t2 = cv::getTextSize(txt2, cv::FONT_HERSHEY_SIMPLEX, 0.65, 2, nullptr);
    cv::putText(frame, txt2,
                cv::Point(btnX + (btnLargura - t2.width) / 2, btnY + (btnAltura + t2.height) / 2),
                cv::FONT_HERSHEY_SIMPLEX, 0.65, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);

    // Botão 3: Como Jogar
    btnY += btnAltura + espaco;
    cv::Rect btn3(btnX, btnY, btnLargura, btnAltura);
    cv::rectangle(frame, btn3, cv::Scalar(0, 120, 180), -1, cv::LINE_AA);
    cv::rectangle(frame, btn3, cv::Scalar(0, 150, 220), 2, cv::LINE_AA);
    std::string txt3 = "[3] Como Jogar";
    cv::Size t3 = cv::getTextSize(txt3, cv::FONT_HERSHEY_SIMPLEX, 0.65, 2, nullptr);
    cv::putText(frame, txt3,
                cv::Point(btnX + (btnLargura - t3.width) / 2, btnY + (btnAltura + t3.height) / 2),
                cv::FONT_HERSHEY_SIMPLEX, 0.65, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);

    // Botão 4: Sair
    btnY += btnAltura + espaco;
    cv::Rect btn4(btnX, btnY, btnLargura, btnAltura);
    cv::rectangle(frame, btn4, cv::Scalar(0, 0, 180), -1, cv::LINE_AA);
    cv::rectangle(frame, btn4, cv::Scalar(0, 0, 220), 2, cv::LINE_AA);
    std::string txt4 = "[4] Sair";
    cv::Size t4 = cv::getTextSize(txt4, cv::FONT_HERSHEY_SIMPLEX, 0.65, 2, nullptr);
    cv::putText(frame, txt4,
                cv::Point(btnX + (btnLargura - t4.width) / 2, btnY + (btnAltura + t4.height) / 2),
                cv::FONT_HERSHEY_SIMPLEX, 0.65, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);

    // Informação da câmara no fundo
    std::string camInfo = "Camera atual: " + std::to_string(cameraId);
    cv::putText(frame, camInfo, cv::Point(10, altura - 15),
                cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(120, 120, 120), 1, cv::LINE_AA);
}

// --- Desenhar menu de configurações ---
void Menu::desenharMenuConfiguracoes(cv::Mat& frame) const {
    frame = cv::Scalar(30, 30, 40);

    int largura = frame.cols;

    // Título
    cv::putText(frame, "Configuracoes", cv::Point(largura / 2 - 130, 80),
                cv::FONT_HERSHEY_DUPLEX, 1.2, cv::Scalar(100, 220, 255), 2, cv::LINE_AA);

    // Listar câmaras disponíveis numa string visual
    cv::putText(frame, "Portas DirectShow Activas no Windows:", cv::Point(50, 130),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(150, 255, 150), 1, cv::LINE_AA);
                
    int startY = 160;
    for (size_t i = 0; i < nomesCamaras.size(); i++) {
        std::string prefixo = (i == cameraId) ? ">>> " : "    ";
        cv::Scalar cor = (i == cameraId) ? cv::Scalar(100, 255, 255) : cv::Scalar(200, 200, 200);
        cv::putText(frame, prefixo + "[" + std::to_string(i) + "] " + nomesCamaras[i], cv::Point(50, startY + i*25),
                    cv::FONT_HERSHEY_SIMPLEX, 0.55, cor, 1, cv::LINE_AA);
    }

    cv::putText(frame, "Pressiona [ESPACO] para alternar a camera", cv::Point(50, 380),
                cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(160, 160, 160), 1, cv::LINE_AA);

    cv::putText(frame, "Pressiona [ESC] para voltar ao menu", cv::Point(50, 420),
                cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(160, 160, 160), 1, cv::LINE_AA);
}

// --- Analisador de Câmaras de Hardware & Virtuais (OBS/Telefones) ---
void Menu::procurarCamaras(cv::Mat& frame, const std::string& nomeJanela) {
    nomesCamaras.clear();
    
    // Feedback visual
    frame = cv::Scalar(20, 20, 30);
    cv::putText(frame, "A obter nomes das cameras virtuais do Windows (DirectShow)...", cv::Point(40, 200),
                cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(200, 255, 200), 1, cv::LINE_AA);
    cv::putText(frame, "Bypass DroidCam/OBS crash ativado...", cv::Point(40, 250),
                cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar(150, 150, 150), 1, cv::LINE_AA);
    cv::imshow(nomeJanela, frame);
    cv::waitKey(50); // Forçar UI update
    
    // O OpenCV highgui já pode ter definido STA, por isso usamos COINIT_APARTMENTTHREADED (ou pedimos para ignorar se já estiver)
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        std::cerr << "[ERRO COM] Falha no CoInitializeEx. HRESULT: " << std::hex << hr << std::dec << std::endl;
        hr = CoInitialize(NULL); // Fallback absoluto antigo do OLE
    }
    
    // Continuar se COM foi inicializado com sucesso ou se apenas avisou RPC_E_CHANGED_MODE (já estava inicializado)
    if (SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE) {
        ICreateDevEnum *pDevEnum = NULL;
        hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));
        if (SUCCEEDED(hr)) {
            IEnumMoniker *pEnum = NULL;
            hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
            if (hr == S_OK) {
                IMoniker *pMoniker = NULL;
                while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {
                    IPropertyBag *pPropBag;
                    hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
                    if (SUCCEEDED(hr)) {
                        // Obter o "FriendlyName" (ex: "DroidCam Video") do BSTR nativo do Windows
                        VARIANT varName;
                        VariantInit(&varName);
                        hr = pPropBag->Read(L"FriendlyName", &varName, 0);
                        if (SUCCEEDED(hr)) {
                            // Extrair o nome usando uma abordagem à prova de bala
                            char charStr[256];
                            WideCharToMultiByte(CP_ACP, 0, varName.bstrVal, -1, charStr, sizeof(charStr), NULL, NULL);
                            
                            std::string camName(charStr);
                            nomesCamaras.push_back(camName);
                            std::cout << "[DSHOW] Encontrou Camera: " << camName << std::endl;
                            
                            VariantClear(&varName);
                        } else {
                            std::cerr << "[AVISO DSHOW] pPropBag->Read falhou ao tentar aceder a FriendlyName!" << std::endl;
                        }
                        pPropBag->Release();
                    } else {
                        std::cerr << "[AVISO DSHOW] pMoniker->BindToStorage falhou!" << std::endl;
                    }
                    pMoniker->Release();
                }
                pEnum->Release();
            } else {
                std::cerr << "[ERRO DSHOW] CreateClassEnumerator devolveu Vazio ou Falhou! HRESULT: " << std::hex << hr << std::dec << std::endl;
            }
            pDevEnum->Release();
        } else {
            std::cerr << "[ERRO DSHOW] CoCreateInstance falhou ao instanciar SystemDeviceEnum. HRESULT: " << std::hex << hr << std::dec << std::endl;
        }
        CoUninitialize();
    }
    
    // Fallback absoluto
    if (nomesCamaras.empty()) {
        std::cerr << "[INFO DSHOW] Nenhuma Camera Directshow amigável extracida no final do loop. Usar Predefinida." << std::endl;
        nomesCamaras.push_back("Camera Predefinida Desconhecida"); 
    }
    
    if (cameraId >= nomesCamaras.size()) {
        cameraId = 0;
    }
}

// --- Executar o loop do menu ---
void Menu::executar() {
    resetar();
    cv::Mat frame(450, 640, CV_8UC3);
    std::string nomeJanela = "HandPaint - Menu";

    cv::namedWindow(nomeJanela, cv::WINDOW_AUTOSIZE | cv::WINDOW_GUI_NORMAL);

    while (!deveIniciar && !deveSair) {
        desenharMenuPrincipal(frame);
        cv::imshow(nomeJanela, frame);

        int tecla = cv::waitKey(30);

        if (tecla == '1') {
            deveIniciar = true;
        } else if (tecla == '2') {
            abrirConfiguracoes();
        } else if (tecla == '3') {
            abrirComoJogar();
        } else if (tecla == '4' || tecla == 27) { // 27 = ESC
            deveSair = true;
        }
    }

    cv::destroyWindow(nomeJanela);
}

// --- Menu de configurações ---
void Menu::abrirConfiguracoes() {
    cv::Mat frame(450, 640, CV_8UC3);
    std::string nomeJanela = "HandPaint - Configuracoes";

    cv::namedWindow(nomeJanela, cv::WINDOW_AUTOSIZE | cv::WINDOW_GUI_NORMAL);

    // Efetuar um Scan ao barramento e controladores virtuais (OBS Camera, Iriun, EpocCam) 
    procurarCamaras(frame, nomeJanela);

    bool sair = false;
    while (!sair) {
        desenharMenuConfiguracoes(frame);
        cv::imshow(nomeJanela, frame);

        int tecla = cv::waitKey(30);

        // [Espaço] para mudar de câmara (loop round-robin na lista das permitidas na máquina)
        if (tecla == ' ' || tecla == 13 || tecla == 32) {
            if (!nomesCamaras.empty()) {
                cameraId = (cameraId + 1) % nomesCamaras.size();
                std::cout << "Camera alternada na interface para: " << nomesCamaras[cameraId] << " (Indice #" << cameraId << ")" << std::endl;
            }
        } else if (tecla == 27) { // ESC para voltar
            sair = true;
        }
    }

    cv::destroyWindow(nomeJanela);
}

// --- Desenhar menu Como Jogar (Tutorial) ---
void Menu::desenharMenuComoJogar(cv::Mat& frame) const {
    frame = cv::Scalar(30, 30, 40);

    int largura = frame.cols;

    // Título
    cv::putText(frame, "Como Jogar", cv::Point(largura / 2 - 100, 45),
                cv::FONT_HERSHEY_DUPLEX, 1.2, cv::Scalar(100, 220, 255), 2, cv::LINE_AA);

    // Instruções Principais
    int startY = 85;
    cv::putText(frame, "COMO PINTAR:", cv::Point(30, startY),
                cv::FONT_HERSHEY_SIMPLEX, 0.65, cv::Scalar(255, 255, 100), 1, cv::LINE_AA);
    cv::putText(frame, "  1. Levanta apenas o Dedo Indicador e aponta.", cv::Point(30, startY + 25),
                cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(200, 255, 200), 1, cv::LINE_AA);
    cv::putText(frame, "  2. Toca fisicamente nos quadrados de cor com a", cv::Point(30, startY + 50),
                cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(200, 255, 200), 1, cv::LINE_AA);
    cv::putText(frame, "     ponta do teu dedo no ar para mudar de tinta.", cv::Point(30, startY + 75),
                cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(200, 255, 200), 1, cv::LINE_AA);

    cv::putText(frame, "COMO APAGAR (BORRACHA):", cv::Point(30, startY + 115),
                cv::FONT_HERSHEY_SIMPLEX, 0.65, cv::Scalar(255, 100, 100), 1, cv::LINE_AA);
    cv::putText(frame, "  - Fecha a mao em Punho! (Recolhe os 4 dedos).", cv::Point(30, startY + 140),
                cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(200, 200, 255), 1, cv::LINE_AA);

    cv::putText(frame, "LIMPAR TELA INTEIRA:", cv::Point(30, startY + 180),
                cv::FONT_HERSHEY_SIMPLEX, 0.65, cv::Scalar(100, 255, 100), 1, cv::LINE_AA);
    cv::putText(frame, "  - Pressiona a tecla [C] no teclado.", cv::Point(30, startY + 205),
                cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(200, 255, 200), 1, cv::LINE_AA);

    cv::putText(frame, "DICA IMPORTANTE PARA A I.A. NAO TRAVAR:", cv::Point(30, startY + 245),
                cv::FONT_HERSHEY_SIMPLEX, 0.65, cv::Scalar(100, 220, 255), 1, cv::LINE_AA);
    cv::putText(frame, "  Mostra APENAS a pele da mao e o braco para a camera!", cv::Point(30, startY + 270),
                cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(180, 180, 180), 1, cv::LINE_AA);
    cv::putText(frame, "  O teu Rosto ou pecas de roupa podem confundir o", cv::Point(30, startY + 295),
                cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(180, 180, 180), 1, cv::LINE_AA);
    cv::putText(frame, "  Rastreio Neural.", cv::Point(30, startY + 320),
                cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(180, 180, 180), 1, cv::LINE_AA);

    cv::putText(frame, "Pressiona [ESC] para voltar ao menu", cv::Point(30, 430),
                cv::FONT_HERSHEY_SIMPLEX, 0.55, cv::Scalar(160, 160, 160), 1, cv::LINE_AA);
}

// --- Menu Como Jogar ---
void Menu::abrirComoJogar() {
    cv::Mat frame(450, 640, CV_8UC3);
    std::string nomeJanela = "HandPaint - Como Jogar";

    cv::namedWindow(nomeJanela, cv::WINDOW_AUTOSIZE | cv::WINDOW_GUI_NORMAL);

    bool sair = false;
    while (!sair) {
        desenharMenuComoJogar(frame);
        cv::imshow(nomeJanela, frame);

        int tecla = cv::waitKey(30);

        if (tecla == 27 || tecla == '3' || tecla == '\b') { // ESC para voltar
            sair = true;
        }
    }

    cv::destroyWindow(nomeJanela);
}

// --- Resetar estado ---
void Menu::resetar() {
    deveIniciar = false;
    deveSair = false;
}
