/**
 * @file viewer.h
 * @brief STL 3Dモデルビューアーのクラス定義
 * @author STL Viewer Team
 * @version 1.0
 */

#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "model_loader.h"
#include "shader.h"

/**
 * @brief 3Dモデルを表示するビューアークラス
 * 
 * このクラスはAssimpを使用して多様な3Dモデル（STL、OBJ、FBX、GLTF等）を読み込み、
 * OpenGLを使用して3Dモデルと座標軸を表示する。
 * カメラはマウススクロールでズーム可能で、ESCキーで終了できる。
 * 
 * 主な機能:
 * - 50+ 3Dモデル形式の読み込み（STL、OBJ、FBX、GLTF等）
 * - 3D座標軸の表示
 * - マウススクロールによるズーム
 * - 自動カメラ配置（モデルが画面中央に表示される）
 * 
 * @note OpenGL 3.3 Core Profileを使用
 * @note GLFWによるウィンドウ管理
 * @note GLMによる数学計算
 * @note Assimpによる3Dモデル読み込み
 */
class STLViewer {
private:
    // ウィンドウ・コンテキスト管理
    std::unique_ptr<GLFWwindow, void(*)(GLFWwindow*)> window;
    
    // シェーダー・描画リソース
    Shader shader;
    ModelMesh mesh;
    
    // OpenGL バッファオブジェクト
    unsigned int axesVAO, axesVBO;      // 座標軸用
    unsigned int modelVAO, modelVBO;    // 3Dモデル用
    
    // カメラシステム
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    
    // ライティングシステム
    glm::vec3 lightPos;
    glm::vec3 lightColor;
    
    // 変換行列
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    
    // プライベートメソッド
    void setupCamera();
    void updateMatrices();
    void updateViewProjectionMatrices();
    void updateModelMatrix();
    void sendMatricesToShader() const;
    bool setupShaders();
    bool setupAxesBuffers();
    bool setupModelBuffers();
    std::vector<float> convertSTLToVertices() const;
    bool createModelBuffers(const std::vector<float>& vertices);
    void setupVertexAttributes(); // 共通の頂点属性設定
    std::vector<float> createAxesVertices() const; // 座標軸頂点データ生成
    bool createAxesOpenGLBuffers(const std::vector<float>& vertices); // 座標軸OpenGLバッファ作成
    bool initializeGLFW();
    bool initializeOpenGL();
    void setupCallbacks();
    void render();
    void renderAxes();
    void renderModel();
    void processInput();

public:
    /**
     * @brief デフォルトコンストラクタ
     * 
     * メンバー変数を初期化する。実際の初期化処理はinit()で行う。
     */
    STLViewer();
    
    /**
     * @brief デストラクタ
     * 
     * OpenGLリソース（VAO、VBO）とGLFWウィンドウを解放する。
     */
    ~STLViewer();

    /**
     * @brief ビューアーを初期化する
     * 
     * GLFW、OpenGL、シェーダー、バッファを初期化し、
     * ウィンドウを作成してOpenGLコンテキストを設定する。
     * 
     * @return 初期化成功時はtrue、失敗時はfalse
     * @pre GLFWが正常にインストールされている
     * @post 成功時は800x600のウィンドウが作成される
     */
    bool init();
    
    /**
     * @brief STLファイルを読み込んで表示用バッファを設定する
     * 
     * 指定されたSTLファイル（ASCII/Binary対応）を読み込み、
     * モデルデータをOpenGLバッファに転送する。モデルは自動的に
     * 原点に配置され、適切なサイズにスケールされる。
     * 
     * @param filename STLファイルのパス（相対パス可）
     * @return 読み込み成功時はtrue、失敗時はfalse
     * @pre init()が正常に完了している
     * @pre filenameが有効なSTLファイルを指している
     * @post 成功時はSTLモデルが表示可能な状態になる
     */
    bool loadSTL(const std::string& filename);
    
    /**
     * @brief メインループを開始する
     * 
     * GLFWのメインループを開始し、ウィンドウが閉じられるまで
     * 描画とイベント処理を継続する。このメソッドはブロッキングで、
     * ウィンドウが閉じられるまで制御を返さない。
     * 
     * @pre init()とloadSTL()が正常に完了している
     * @post ウィンドウが閉じられると制御が戻る
     */
    void run();

    /**
     * @brief マウススクロールコールバック関数をフレンドとして宣言
     * 
     * GLFWのスクロールコールバックからカメラのズーム機能にアクセスするため。
     */
    friend void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

private:
    /**
     * @brief OpenGLバッファのペア（VAO + VBO）
     */
    struct BufferPair {
        unsigned int VAO;
        unsigned int VBO;
    };
    
    /**
     * @brief 頂点データからOpenGLバッファ（VAO + VBO）を作成する
     * 
     * 共通のバッファ作成ロジックを提供し、コードの重複を排除する。
     * 
     * @param vertices 頂点データ
     * @return 作成されたVAOとVBOのペア
     */
    BufferPair createOpenGLBuffers(const std::vector<float>& vertices);
    
    /**
     * @brief エラーメッセージをログに出力する
     * 
     * @param message エラーメッセージ
     * @param functionName エラーが発生した関数名
     */
    void logError(const std::string& message, const std::string& functionName = "") const;
};