# HandPaint - Pintura Digital com Inteligência Artificial 🎨✋

HandPaint é uma aplicação interativa desenvolvida em **C++** que utiliza Vision Compute e Inteligência Artificial para permitir que o utilizador pinte digitalmente no ecrã utilizando apenas movimentos das mãos captados pela webcam.

## 🚀 Funcionalidades

- **Rastreio de Mão em Tempo Real:** Utiliza modelos de Deep Learning (MediaPipe) para detetar 21 pontos-chave (landmarks) da mão com alta precisão.
- **Pintura Intuitiva:** 
  - Levante apenas o **Dedo Indicador** para começar a desenhar.
  - Alterne entre 5 cores vibrantes e uma borracha tocando nos botões virtuais no topo do ecrã.
- **Borracha em Punho:** Feche a mão em formato de **Punho** (escondendo os dedos) para ativar a borracha dinâmica. O tamanho da borracha adapta-se à proximidade da mão.
- **Gestão de Desenhos:** Atalhos para Guardar (`S`), Carregar (`L`) e Limpar (`C`) a tela de desenho.
- **Interface Polida:** Menu de entrada com instruções, definições de câmara e visual limpo (sem janelas de consola CMD).

## 🛠️ Tecnologias Utilizadas

- **Linguagem:** C++17
- **Visão Computacional:** [OpenCV 4.10+](https://opencv.org/)
- **Inteligência Artificial:** [Google MediaPipe Hand Tracking (TFLite)](https://google.github.io/mediapipe/solutions/hands.html)
- **Interface de SO:** Windows API (DirectShow para gestão de câmaras)
- **Compilador:** GCC (via MSYS2 UCRT64)

## 🧠 Como funciona o rastreio?

O núcleo do sistema reside na classe `HandTracker`, que implementa um pipeline de duas fases:
1. **Palm Detection:** Uma rede neuronal leve localiza a palma da mão no frame total.
2. **Hand Landmarks (MediaPipe v8):** Uma rede neuronal completa extrai 21 pontos 3D da mão.
O sistema utiliza uma técnica de **ROI (Region of Interest) Tracking**: após a primeira deteção, a IA foca-se apenas na área onde a mão estava, permitindo maior FPS e fluidez. Se o rastreio falhar ou for perdido, o sistema volta automaticamente à deteção global.

## 📥 Download Plug & Play

Devido ao tamanho das bibliotecas (DLLs do OpenCV e Qt), o pacote completo de distribuição (**117MB**) não pode ser carregado diretamente no código fonte do GitHub (limite de 100MB). 

Podes descarregar a versão pronta a usar em:
- [HandPaint v1.0 - Releases (Brevemente)](#) 

**Nota:** Basta extrair o ZIP e executar o ficheiro `Iniciar_Jogo.bat`. Não é necessário instalar nada!

## 🔧 Como Compilar (Para Desenvolvedores)

Requisitos: **MSYS2** com o ambiente **UCRT64** e a biblioteca `opencv` instalada (`pacman -S mingw-w64-ucrt-x86_64-opencv`).

1. Clone o repositório.
2. Execute o script de build:
   ```cmd
   build.bat
   ```
3. O executável `HandPaint.exe` será gerado na raiz.

## 📚 Créditos e Referências

Este projeto foi construído sobre a fundação do repositório [mediapipe_hand_tracking_cpp](https://github.com/homuler/mediapipe_hand_tracking_cpp), adaptando os modelos TFLite para uma arquitetura orientada a objetos personalizada para o ambiente Windows.

---
Desenvolvido por **Mateus Costa** como parte do Trabalho de Recuperação de TLP. 🌟
