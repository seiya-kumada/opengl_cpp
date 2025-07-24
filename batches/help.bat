@echo off
echo ========================================
echo STL Viewer Help
echo ========================================
echo.

if exist "build\Release\stl_viewer.exe" (
    echo Running built-in help (Release build)...
    echo.
    build\Release\stl_viewer.exe --help
) else if exist "build\Debug\stl_viewer.exe" (
    echo Running built-in help (Debug build)...
    echo.
    build\Debug\stl_viewer.exe --help
) else (
    echo Built-in help not available (executable not found).
    echo Please run build_release.bat or build_debug.bat first.
    echo.
    echo Manual usage information:
    echo.
    echo Available batch files:
    echo   build_release.bat      - Build the project (Release)
    echo   build_debug.bat        - Build the project (Debug)
    echo   clean_build.bat        - Clean build (remove build dir first)
    echo   run.bat [args]         - Run the viewer with arguments (auto-detect)
    echo   run_release.bat [args] - Run the viewer (Release build)
    echo   run_debug.bat [args]   - Run the viewer (Debug build)
    echo   help.bat               - Show this help
    echo.
    echo Command line options:
    echo   --help              Show help message
    echo   --window-size W H   Set window size
    echo   --fullscreen        Start in fullscreen mode
    echo   --vsync             Enable vertical synchronization
    echo   --verbose           Enable verbose output
    echo.
    echo Examples:
    echo   run.bat stls\sample.stl
    echo   run_release.bat stls\sample.stl --window-size 1024 768
    echo   run_debug.bat stls\sample.stl --verbose --fullscreen
)

echo.
pause 