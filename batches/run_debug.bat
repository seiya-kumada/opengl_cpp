@echo off
echo ========================================
echo STL Viewer Run Script (Debug)
echo ========================================

REM Debug実行ファイルの存在確認
if not exist "build\Debug\stl_viewer.exe" (
    echo Error: Debug executable not found.
    echo Please run build_debug.bat first.
    echo Expected location: build\Debug\stl_viewer.exe
    pause
    exit /b 1
)

REM 引数が指定されているかチェック
if "%1"=="" (
    echo Usage: run_debug.bat [STL_FILE_PATH] [OPTIONS]
    echo.
    echo Examples:
    echo   run_debug.bat stls\sample.stl
    echo   run_debug.bat stls\sample.stl --window-size 1024 768
    echo   run_debug.bat stls\sample.stl --verbose --fullscreen
    echo.
    echo Available options:
    echo   --help              Show help message
    echo   --window-size W H   Set window size
    echo   --fullscreen        Start in fullscreen mode
    echo   --vsync             Enable vertical synchronization
    echo   --verbose           Enable verbose output
    echo.
    pause
    exit /b 1
)

REM Debug実行ファイルを実行
echo Running STL Viewer (Debug build)...
echo Command: build\Debug\stl_viewer.exe %*
echo.

build\Debug\stl_viewer.exe %*

echo.
echo STL Viewer execution completed.
pause 