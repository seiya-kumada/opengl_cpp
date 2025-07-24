@echo off
echo ========================================
echo STL Viewer Debug Build Script
echo ========================================

REM ビルドディレクトリの作成
if not exist build mkdir build
cd build

REM CMakeの設定（Debug）
echo Configuring CMake for Debug build...
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/local/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Debug
if %ERRORLEVEL% neq 0 (
    echo Error: CMake configuration failed
    pause
    exit /b 1
)

REM Debugビルドの実行
echo Building project in Debug mode...
cmake --build . --config Debug
if %ERRORLEVEL% neq 0 (
    echo Error: Debug build failed
    pause
    exit /b 1
)

echo.
echo ========================================
echo Debug build completed successfully!
echo Executable: build\Debug\stl_viewer.exe
echo ========================================
echo.

REM ビルドディレクトリから戻る
cd ..

pause 