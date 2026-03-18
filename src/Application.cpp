#include "Application.h"
#include <iostream>
#include <string>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

// ============================================================
// Implementação da classe Application
// ============================================================

Application::Application(const std::string& caminhoModelos)
    : estadoAtual(MENU), tracker(nullptr), canvas(nullptr),
      overlay(nullptr), cameraId(0), caminhoModelos(caminhoModelos),
      larguraJanela(640), alturaJanela(480), estavaAPintar(false), estavaAApagar(false) {
}

Application::~Application() {
    libertarComponentes();
}

// --- Inicializar componentes (new - requisito 4) ---
void Application::inicializarComponentes() {
    // Alocar com new (apontadores - requisito 4)
    tracker = new HandTracker(caminhoModelos);
    canvas = new Canvas(larguraJanela, alturaJanela);
    overlay = new UIOverlay(alturaJanela, canvas->getPaleta());

    std::cout << "Componentes inicializados com sucesso!" << std::endl;
}

// --- Libertar componentes (delete - requisito 4) ---
void Application::libertarComponentes() {
    if (tracker != nullptr) {
        delete tracker;
        tracker = nullptr;
    }
    if (canvas != nullptr) {
        delete canvas;
        canvas = nullptr;
    }
    if (overlay != nullptr) {
        delete overlay;
        overlay = nullptr;
    }
}

// --- Pedir nome do ficheiro via consola (requisito 7: validação input) ---
std::string Application::pedirNomeFicheiro() const {
    std::string nome;
    bool valido = false;

    while (!valido) {
        std::cout << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "  Introduza o nome do ficheiro (.txt):" << std::endl;
        std::cout << "  Exemplo: meu_desenho.txt" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "> ";
        std::getline(std::cin, nome);

        valido = validarNomeFicheiro(nome);

        if (!valido) {
            std::cout << "Erro: Nome invalido! O ficheiro deve:" << std::endl;
            std::cout << "  - Nao estar vazio" << std::endl;
            std::cout << "  - Terminar com .txt" << std::endl;
            std::cout << "  - Ter pelo menos 5 caracteres (ex: a.txt)" << std::endl;
        }
    }

    return nome;
}

// --- Validar nome do ficheiro (requisito 7) ---
bool Application::validarNomeFicheiro(const std::string& nome) const {
    if (nome.empty()) return false;
    if (nome.length() < 5) return false;

    // Verificar se termina com .txt
    std::string extensao = ".txt";
    if (nome.length() < extensao.length()) return false;

    return nome.compare(nome.length() - extensao.length(),
                        extensao.length(), extensao) == 0;
}

// --- Processar input do teclado durante a pintura ---
void Application::processarInput(int tecla) {
    if (tecla == 27) { // ESC - Voltar ao menu
        estadoAtual = MENU;
    } else if (tecla == 's' || tecla == 'S') { // Guardar
        std::string nomeFicheiro = pedirNomeFicheiro();
        if (canvas) {
            canvas->guardarFicheiro(nomeFicheiro);
        }
    } else if (tecla == 'l' || tecla == 'L') { // Carregar
        std::string nomeFicheiro = pedirNomeFicheiro();
        if (canvas) {
            canvas->carregarFicheiro(nomeFicheiro);
        }
    } else if (tecla == 'c' || tecla == 'C') { // Limpar tela
        if (canvas) {
            canvas->limparTela();
            estavaAPintar = false;  // Resetar estado para poder pintar novamente
            std::cout << "Tela limpa!" << std::endl;
        }
    }
}

// --- Ciclo principal de pintura ---
void Application::cicloPintura() {
    cameraId = menu.getCameraId();

    std::cout << "[Video Stream] A Iniciar Camera " << cameraId << "..." << std::endl;
    // Abertura simples sem restrições de backend — permite que o Windows 
    // escolha o canal correto (Phone Link, DirectShow, etc.)
    camera.open(cameraId);

    if (!camera.isOpened()) {
        std::cerr << "Erro: Nao foi possivel abrir a camera " << cameraId << std::endl;
        std::cerr << "Verifique se a camera esta conectada." << std::endl;
        return;
    }

    // Otimização de Performance HGW: Forçar resolução standard (640x480)
    // Resoluções altas (ex: 1080p) reduzem drasticamente o FPS por causa da USB
    // Menos FPS = O Fast Motion Tracking quebra por a mão saltar muito entre frames
    /* 
    camera.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    camera.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    camera.set(cv::CAP_PROP_FPS, 30);
    */

    // Obter dimensões REAIS da câmara (após set)
    larguraJanela = static_cast<int>(camera.get(cv::CAP_PROP_FRAME_WIDTH));
    alturaJanela = static_cast<int>(camera.get(cv::CAP_PROP_FRAME_HEIGHT));

    // Inicializar componentes
    inicializarComponentes();

    std::string nomeJanela = "HandPaint - A Pintar";
    cv::namedWindow(nomeJanela, cv::WINDOW_AUTOSIZE | cv::WINDOW_GUI_NORMAL);

    std::cout << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "  HandPaint - Modo Pintura" << std::endl;
    std::cout << "  Controlos:" << std::endl;
    std::cout << "  [ESC] - Voltar ao menu" << std::endl;
    std::cout << "  [S]   - Guardar desenho" << std::endl;
    std::cout << "  [L]   - Carregar desenho" << std::endl;
    std::cout << "  [C]   - Limpar tela" << std::endl;
    std::cout << "========================================" << std::endl;

    while (estadoAtual == PINTURA) {
        cv::Mat frame;
        bool sucesso = camera.read(frame);

        if (!sucesso || frame.empty()) {
            std::cerr << "Erro ao ler frame da camera." << std::endl;
            break;
        }

        // Espelhar a imagem (efeito espelho)
        cv::flip(frame, frame, 1);

        // Processar o hand tracking
        tracker->processar(frame);

        // Verificar se o dedo está sobre um botão de cor
        if (tracker->isMaoDetectada()) {
            cv::Point indicador = tracker->getPontaIndicador();

            // Verificar clique em botão de cor
            int botaoClicado = overlay->verificarCliqueBotao(indicador);
            if (botaoClicado >= 0) {
                overlay->selecionarBotao(botaoClicado);
            }

            // Se está a apontar (só indicador levantado) E O dedo
            // NÃO está sobre a barra de cores (que agora está no topo), desenhar
            bool naBarraCores = indicador.y < overlay->getLarguraBarra();

            if (tracker->isApagando()) {
                // Se mudou de Pintar para Apagar repentinamente, resetar estado antigo
                if (estavaAPintar) {
                    estavaAPintar = false; 
                }

                if (!estavaAApagar) {
                    canvas->iniciarNovoTraco(
                        cv::Scalar(0,0,0), 
                        tracker->getRaioBorracha() * 2, // Espessura dinâmica baseada na Mão Aberta
                        "Borracha", 
                        true
                    );
                    estavaAApagar = true;
                }
                canvas->adicionarPontoAoTracoAtual(tracker->getCentroBorracha());
                
            } else if (tracker->isPintando() && !naBarraCores) {
                // Se mudou de Apagar para Pintar repentinamente, resetar estado da borracha
                if (estavaAApagar) {
                    estavaAApagar = false;
                }

                if (!estavaAPintar) {
                    // Começar um novo traço
                    canvas->iniciarNovoTraco(
                        overlay->getCorSelecionada(),
                        5,
                        overlay->getNomeCorSelecionada(),
                        false
                    );
                    estavaAPintar = true;
                }
                canvas->adicionarPontoAoTracoAtual(indicador);
            } else {
                estavaAPintar = false;
                estavaAApagar = false;
            }
        } else {
            estavaAPintar = false;
            estavaAApagar = false;
        }

        // Renderizar o canvas sobre o frame da câmara
        cv::Mat canvasImg = canvas->renderizar();

        // Criar a imagem combinada (frame + canvas)
        cv::Mat resultado = frame.clone();

        // Sobrepor o canvas ao frame (onde há cor no canvas)
        for (int y = 0; y < resultado.rows; y++) {
            for (int x = 0; x < resultado.cols; x++) {
                cv::Vec3b pixel = canvasImg.at<cv::Vec3b>(y, x);
                if (pixel[0] != 0 || pixel[1] != 0 || pixel[2] != 0) {
                    resultado.at<cv::Vec3b>(y, x) = pixel;
                }
            }
        }

        // Desenhar o UI overlay
        overlay->desenharIndicadorEstado(resultado, tracker->isMaoDetectada());
        overlay->desenharBarraCores(resultado);

        // Desenhar landmarks de debug
        tracker->desenharLandmarks(resultado);

        // Se mão detetada, desenhar cursor
        if (tracker->isMaoDetectada()) {
            if (tracker->isApagando()) {
                // Desenhar Feedback visual (Vulto semi-transparente que representa o apagar)
                cv::Mat mascaraApagar = resultado.clone();
                cv::circle(mascaraApagar, tracker->getCentroBorracha(), tracker->getRaioBorracha(), cv::Scalar(150, 150, 150), -1, cv::LINE_AA);
                cv::addWeighted(mascaraApagar, 0.35, resultado, 0.65, 0, resultado);
                // Borda da borracha na UI
                cv::circle(resultado, tracker->getCentroBorracha(), tracker->getRaioBorracha(), cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
            } else {
                cv::Point indicador = tracker->getPontaIndicador();
                overlay->desenharCursor(resultado, indicador, overlay->getCorSelecionada());
            }
        }

        // Mostrar o resultado
        cv::imshow(nomeJanela, resultado);

        // Processar input do teclado
        int tecla = cv::waitKey(1);
        if (tecla >= 0) {
            processarInput(tecla);
        }
    }

    // Cleanup
    camera.release();
    cv::destroyWindow(nomeJanela);
    libertarComponentes();
}

// --- Método principal ---
void Application::executar() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Bem-vindo ao HandPaint!" << std::endl;
    std::cout << "  Pinta com as tuas maos!" << std::endl;
    std::cout << "========================================" << std::endl;

    while (estadoAtual != A_SAIR) {
        if (estadoAtual == MENU) {
            menu.executar();

            if (menu.getDeveSair()) {
                estadoAtual = A_SAIR;
            } else if (menu.getDeveIniciar()) {
                estadoAtual = PINTURA;
                cicloPintura();
            }
        }
    }

    std::cout << "Obrigado por usares o HandPaint! Ate a proxima!" << std::endl;
}
