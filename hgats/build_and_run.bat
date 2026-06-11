@echo off

echo ================================
echo   Building HGATS Project...
echo ================================

cd /d "D:\Graduation Project\HGATS_CPP_V2"

echo.
echo [1/3] Running CMake configure...
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

echo.
echo [2/3] Building project...
cmake --build build

echo.
echo [3/3] Running executable...
cd build
HGATS.exe

echo.
echo ================================
echo   Done!
echo ================================

pause