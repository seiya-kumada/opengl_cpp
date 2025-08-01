/**
 * @file main.cpp
 * @brief STL Viewer アプリケーションのエントリーポイント
 * @author STL Viewer Team
 * @version 1.0
 */

#include <boost/program_options.hpp>
#include <cstdlib>
#include <filesystem>
#include <iostream>

#include "viewer.h"

namespace po = boost::program_options;

// アプリケーション定数
namespace {
    constexpr int DEFAULT_WINDOW_WIDTH = 800;   ///< デフォルトウィンドウ幅
    constexpr int DEFAULT_WINDOW_HEIGHT = 600;  ///< デフォルトウィンドウ高
    constexpr const char* STL_EXTENSION = ".stl"; ///< STLファイル拡張子
}

/**
 * @brief ビューアーの設定を格納する構造体
 */
struct ViewerConfig
{
    std::string stlFilePath; ///< STLファイルのパス
    int windowWidth = DEFAULT_WINDOW_WIDTH;   ///< ウィンドウ幅（ピクセル）
    int windowHeight = DEFAULT_WINDOW_HEIGHT;  ///< ウィンドウ高（ピクセル）
};

/**
 * @brief 使用方法とヘルプメッセージを表示する
 *
 * @param desc コマンドラインオプションの説明
 * @param programName プログラム名
 */
void printUsage(const po::options_description &desc, const char *programName)
{
    std::cout << "Usage: " << programName << " [options] <STL_FILE_PATH>" << std::endl;
    std::cout << desc << std::endl;
    std::cout << "Example: " << programName << " model.stl" << std::endl;
    std::cout << std::endl;
    std::cout << "Supported formats:" << std::endl;
    std::cout << "  - ASCII STL" << std::endl;
    std::cout << "  - Binary STL" << std::endl;
}

/**
 * @brief STLファイルの妥当性を検証する
 *
 * ファイルの存在、サイズ、拡張子をチェックする。
 * 拡張子が.stlでない場合は警告を表示するが処理を継続する。
 *
 * @param filePath 検証するSTLファイルのパス
 * @return 検証成功時はtrue、失敗時はfalse
 */
bool validateSTLFile(const std::string &filePath)
{
    // ファイルの存在チェック
    if (!std::filesystem::exists(filePath))
    {
        std::cerr << "Error: STL file does not exist: " << filePath << std::endl;
        return false;
    }

    // ファイルサイズチェック
    auto path = std::filesystem::path{filePath};
    if (std::filesystem::file_size(path) == 0)
    {
        std::cerr << "Error: STL file is empty: " << filePath << std::endl;
        return false;
    }

    // ファイル拡張子チェック
    if (path.extension().string() != STL_EXTENSION)
    {
        std::cerr << "Warning: File does not have " << STL_EXTENSION << " extension: " << filePath << std::endl;
        std::cerr << "Warning: Continuing anyway..." << std::endl;
    }

    return true;
}

/**
 * @brief コマンドライン引数を解析して設定に格納する
 *
 * Boost.Program_optionsを使用してコマンドライン引数を解析し、
 * ViewerConfig構造体に設定値を格納する。
 *
 * @param argc 引数の数
 * @param argv 引数の配列
 * @param config [out] 解析結果を格納する設定構造体
 * @return 解析成功時はtrue、失敗またはヘルプ表示時はfalse
 */
bool parseCommandLine(int argc, char *argv[], ViewerConfig &config)
{
    auto desc = po::options_description{"STL Viewer Options"};
    desc.add_options()("help,h", "Show this help message")("stl-file", po::value<std::string>(), "STL file path");

    auto p = po::positional_options_description{};
    p.add("stl-file", 1);

    auto vm = po::variables_map{};

    try
    {
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);
    }
    catch (const po::error &e)
    {
        std::cerr << "Error parsing command line: " << e.what() << std::endl;
        printUsage(desc, argv[0]);
        return false;
    }

    // ヘルプ表示
    if (vm.count("help"))
    {
        printUsage(desc, argv[0]);
        return false;
    }

    // STLファイルパスのチェック
    if (!vm.count("stl-file"))
    {
        std::cerr << "Error: STL file path is required." << std::endl;
        printUsage(desc, argv[0]);
        return false;
    }

    config.stlFilePath = vm["stl-file"].as<std::string>();
    return true;
}

/**
 * @brief STLビューアーを初期化してSTLファイルを読み込む
 *
 * ビューアーの初期化とSTLファイルの読み込みを行う。
 * どちらかが失敗した場合はエラーメッセージを表示する。
 *
 * @param config ビューアーの設定
 * @param viewer 初期化するSTLビューアーオブジェクト
 * @return 初期化と読み込みが成功した場合はtrue、失敗した場合はfalse
 */
bool initializeViewer(const ViewerConfig &config, STLViewer &viewer)
{
    if (!viewer.init())
    {
        std::cerr << "Error: Failed to initialize STL Viewer" << std::endl;
        return false;
    }

    if (!viewer.loadSTL(config.stlFilePath))
    {
        std::cerr << "Error: Failed to load STL file" << std::endl;
        return false;
    }

    return true;
}

/**
 * @brief アプリケーションのメイン関数
 *
 * STL Viewerアプリケーションのエントリーポイント。
 * コマンドライン引数を解析し、STLファイルを検証した後、
 * ビューアーを初期化してメインループを実行する。
 *
 * @param argc コマンドライン引数の数
 * @param argv コマンドライン引数の配列
 * @return プログラムの終了コード（EXIT_SUCCESS または EXIT_FAILURE）
 */
int main(int argc, char *argv[])
{
    auto config = ViewerConfig{};

    // コマンドライン解析
    if (!parseCommandLine(argc, argv, config))
    {
        return EXIT_FAILURE;
    }

    // STLファイルの検証
    if (!validateSTLFile(config.stlFilePath))
    {
        std::cerr << "Error: Failed to validate STL file" << std::endl;
        return EXIT_FAILURE;
    }

    // ビューアーの初期化
    auto viewer = STLViewer{};
    if (!initializeViewer(config, viewer))
    {
        return EXIT_FAILURE;
    }

    // メインループを開始
    viewer.run();

    return EXIT_SUCCESS;
}