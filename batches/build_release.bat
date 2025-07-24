@echo off
echo ========================================
echo STL Viewer Release Build Script
echo ========================================

REM ビルドディレクトリの作成
if not exist build mkdir build
cd build

REM CMakeの設定（Release）
echo Configuring CMake for Release build...
cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/local/vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% neq 0 (
    echo Error: CMake configuration failed
    pause
    exit /b 1
)

REM Releaseビルドの実行
echo Building project in Release mode...
cmake --build . --config Release
if %ERRORLEVEL% neq 0 (
    echo Error: Release build failed
    pause
    exit /b 1
)

echo.
echo ========================================
echo Release build completed successfully!
echo Executable: build\Release\stl_viewer.exe
echo ========================================
echo.

REM ビルドディレクトリから戻る
cd ..

pause 