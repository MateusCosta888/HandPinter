# HandPaint - Digital Painting with Artificial Intelligence 🎨✋

HandPaint is an interactive application developed in **modern C++** that uses Computer Vision and AI to allow users to paint, draw, and erase on screen using only hand gestures captured via webcam.

## 🚀 Key Features

- **Real-Time Hand Tracking:** Uses Deep Learning models (MediaPipe) to detect 21 hand landmarks with high precision.
- **Intuitive Painting Mode:** 
  - Raise only the **Index Finger** to start drawing.
  - Switch between 5 vibrant colors and an eraser by touching virtual buttons at the top of the screen.
- **Fist Gesture Eraser:** Close your hand into a **Fist** (retracting all fingers) to activate the dynamic eraser. The eraser size adapts based on the hand's distance to the camera.
- **Drawing Management:** Fast shortcuts to Save (`S`), Load (`L`), and Clear (`C`) the canvas.
- **Polished Interface:** Main menu with instructions, camera settings, and a clean UI (no CMD/Console windows).

## 🛠️ Technology Stack

- **Language:** C++17
- **Computer Vision:** [OpenCV 4.10+](https://opencv.org/)
- **Artificial Intelligence:** [Google MediaPipe Hand Tracking (TFLite)](https://google.github.io/mediapipe/solutions/hands.html)
- **OS Integration:** Windows API (DirectShow for camera management)
- **Compiler:** GCC (via MSYS2 UCRT64)

## 🧠 How the tracking works

The core of the system is the `HandTracker` class, which implements a specialized 2-stage pipeline:
1. **Palm Detection:** A lightweight neural network locates the palm's bounding box in the full frame.
2. **Hand Landmarks (MediaPipe v8):** A full-capacity neural network extracts 21 three-dimensional points from the hand.
The system utilizes a **ROI (Region of Interest) Tracking** technique: once detected, the AI focuses only on the area where the hand was in the previous frame, allowing for higher FPS and smoother movement. If tracking is lost, the system automatically falls back to global detection.

## 📥 Plug & Play Download

Due to the size of the dependencies (OpenCV and Qt DLLs), the full distribution package (**~117MB**) cannot be uploaded directly to the GitHub source code repository (100MB limit).

You can download the ready-to-use version at:
- [HandPaint v1.0 - Releases](#)

**Note:** Simply extract the ZIP and run `Iniciar_Jogo.bat`. No installation required!

## 🔧 How to Compile (For Developers)

Requirements: **MSYS2** with the **UCRT64** environment and the `opencv` package installed (`pacman -S mingw-w64-ucrt-x86_64-opencv`).

1. Clone the repository.
2. Run the build script:
   ```cmd
   build.bat
   ```
3. The `HandPaint.exe` executable will be generated in the root directory.

## 📚 Credits and References

This project was built upon the foundation of the [mediapipe_hand_tracking_cpp](https://github.com/homuler/mediapipe_hand_tracking_cpp) repository, adapting the TFLite models for a custom object-oriented architecture suited for the Windows environment.

---
Developed by **Mateus Costa**. 🌟
