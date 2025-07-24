@echo off
echo ========================================
echo STL Viewer Run Script (Release)
echo ========================================

REM Release実行ファイルの存在確認
if not exist "build\Release\stl_viewer.exe" (
    echo Error: Release executable not found.
    echo Please run build_release.bat or build.bat first.
    echo Expected location: build\Release\stl_viewer.exe
    pause
    exit /b 1
)

REM 引数が指定されているかチェック
if "%1"=="" (
    echo Usage: run_release.bat [STL_FILE_PATH] [OPTIONS]
    echo.
    echo Examples:
    echo   run_release.bat stls\sample.stl
    echo   run_release.bat stls\sample.stl --window-size 1024 768
    echo   run_release.bat stls\sample.stl --verbose --fullscreen
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

REM Release実行ファイルを実行
echo Running STL Viewer (Release build)...
echo Command: build\Release\stl_viewer.exe %*
echo.

build\Release\stl_viewer.exe %*

echo.
echo STL Viewer execution completed.
pause 