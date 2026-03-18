@echo off
REM Script de compilacao do HandPaint
REM Usa g++ do MSYS2 com OpenCV

set PATH=C:\msys64\ucrt64\bin;%PATH%

echo ========================================
echo   Compilando HandPaint...
echo ========================================

g++ -std=c++17 -O2 ^
    src/main.cpp ^
    src/Application.cpp ^
    src/Menu.cpp ^
    src/HandTracker.cpp ^
    src/Canvas.cpp ^
    src/Stroke.cpp ^
    src/UIOverlay.cpp ^
    -IC:/msys64/ucrt64/include/opencv4 ^
    -LC:/msys64/ucrt64/lib ^
    -lopencv_highgui -lopencv_imgproc -lopencv_core -lopencv_videoio -lopencv_dnn -lopencv_video -lopencv_objdetect ^
    -lole32 -loleaut32 -luuid -lstrmiids ^
    -mwindows ^
    -o HandPaint.exe

if %ERRORLEVEL% EQU 0 (
    echo.
    echo   Compilacao concluida com sucesso!
    echo   Executavel: HandPaint.exe
    echo ========================================
) else (
    echo.
    echo   ERRO na compilacao!
    echo ========================================
)
