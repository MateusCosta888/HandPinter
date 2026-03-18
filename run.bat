@echo off
REM ============================================
REM   HandPaint - Script de Lancamento
REM   Configura o PATH e executa a aplicacao
REM ============================================

set PATH=C:\msys64\ucrt64\bin;%PATH%
set QT_QPA_PLATFORM_PLUGIN_PATH=C:\msys64\ucrt64\share\qt6\plugins\platforms

echo ========================================
echo   HandPaint - A Iniciar...
echo ========================================

HandPaint.exe %*

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo   Ocorreu um erro ao executar o HandPaint.
    echo   Codigo de erro: %ERRORLEVEL%
)

pause
