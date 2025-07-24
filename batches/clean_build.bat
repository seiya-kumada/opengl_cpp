@echo off
echo ========================================
echo STL Viewer Clean Build Script
echo ========================================

REM ビルドディレクトリの削除確認
if exist build (
    echo Removing existing build directory...
    rmdir /s /q build
    if %ERRORLEVEL% neq 0 (
        echo Error: Failed to remove build directory
        pause
        exit /b 1
    )
    echo Build directory removed.
)

echo.
echo Starting clean build...
echo.

REM ビルドスクリプトを呼び出し
call build_release.bat

echo.
echo ========================================
echo Clean build completed!
echo ======================================== 