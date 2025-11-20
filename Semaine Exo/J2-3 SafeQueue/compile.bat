@echo off
echo === Compilation TestSafeQueue ===

REM Chercher cl.exe (Visual Studio)
where cl.exe >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Compilation avec MSVC (cl.exe)...
    cl.exe /EHsc /W4 main.cpp /Fe:TestSafeQueue.exe
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo ✓ Compilation reussie : TestSafeQueue.exe
        echo.
        echo Lancer le programme ? (Y/N)
        choice /C YN /N
        if errorlevel 2 goto end
        if errorlevel 1 TestSafeQueue.exe
    ) else (
        echo ✗ Erreur de compilation
    )
    goto end
)

REM Chercher g++ (MinGW)
where g++ >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Compilation avec g++...
    g++ -o TestSafeQueue.exe main.cpp -std=c++11 -Wall
    if %ERRORLEVEL% EQU 0 (
        echo.
        echo ✓ Compilation reussie : TestSafeQueue.exe
        echo.
        echo Lancer le programme ? (Y/N)
        choice /C YN /N
        if errorlevel 2 goto end
        if errorlevel 1 TestSafeQueue.exe
    ) else (
        echo ✗ Erreur de compilation
    )
    goto end
)

echo.
echo ✗ Aucun compilateur trouve (cl.exe ou g++)
echo.
echo Solutions :
echo   1. Ouvrir "Developer Command Prompt for VS" et relancer
echo   2. Installer MinGW-w64 et ajouter au PATH
echo   3. Ouvrir le projet dans Visual Studio

:end
pause
