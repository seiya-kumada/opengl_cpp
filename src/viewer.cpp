#include "viewer.h"
#include "model_loader.h"
#include <iostream>
#include <sstream>
#include <array>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/join.hpp>
#include <boost/range/algorithm/copy.hpp>

// 内部定数定義
namespace
{
// ウィンドウ設定
constexpr int WINDOW_WIDTH{800};
constexpr int WINDOW_HEIGHT{600};
constexpr int OPENGL_VERSION_MAJOR{3};
constexpr int OPENGL_VERSION_MINOR{3};

// カメラ設定
constexpr float FOV_DEGREES{45.0f};
constexpr float NEAR_PLANE{0.1f};
constexpr float FAR_PLANE{100.0f};
constexpr float CAMERA_DISTANCE{8.0f};

// 描画設定
constexpr float AXIS_LENGTH{2.0f};
constexpr float LINE_WIDTH{3.0f};
constexpr float MODEL_DESIRED_SIZE{1.5f};
constexpr float SCROLL_SENSITIVITY{0.3f};

// 色設定
constexpr float MODEL_COLOR_R{0.8f};
constexpr float MODEL_COLOR_G{0.8f};
constexpr float MODEL_COLOR_B{0.8f};

constexpr float BACKGROUND_R{0.2f};
constexpr float BACKGROUND_G{0.2f};
constexpr float BACKGROUND_B{0.2f};

// ライティング設定
constexpr float AMBIENT_STRENGTH{0.1f};   // 環境光強度
constexpr float SPECULAR_STRENGTH{0.5f};  // 鏡面光強度
constexpr float SHININESS{32.0f};         // 光沢度

// シェーダーファイルパス
constexpr const char* VERTEX_SHADER_PATH{"shaders/vertex.glsl"};
constexpr const char* FRAGMENT_SHADER_PATH{"shaders/fragment.glsl"};

// 頂点データ構造
constexpr int VERTEX_COMPONENTS{9}; // 位置3 + 色3 + 法線3
constexpr int NORMAL_COMPONENTS{3}; // 法線ベクトルの要素数
constexpr int POSITION_COMPONENTS{3};
constexpr int COLOR_COMPONENTS{3};
constexpr int TRIANGLE_VERTICES{3};
constexpr int AXES_COUNT{3};
constexpr int AXIS_VERTICES{2}; // 原点と先端

// 頂点属性インデックス（シェーダーのlocation番号）
constexpr int POSITION_ATTRIBUTE_INDEX{0};
constexpr int COLOR_ATTRIBUTE_INDEX{1};
constexpr int NORMAL_ATTRIBUTE_INDEX{2};
} // namespace

STLViewer::STLViewer() : window(nullptr, glfwDestroyWindow), axesVAO(0), axesVBO(0), modelVAO(0), modelVBO(0)
{
}

STLViewer::~STLViewer()
{
    // 座標軸用のリソースを削除
    if (axesVAO != 0)
    {
        glDeleteVertexArrays(1, &axesVAO);
    }
    if (axesVBO != 0)
    {
        glDeleteBuffers(1, &axesVBO);
    }

    // 3Dモデル用のリソースを削除
    if (modelVAO != 0)
    {
        glDeleteVertexArrays(1, &modelVAO);
    }
    if (modelVBO != 0)
    {
        glDeleteBuffers(1, &modelVBO);
    }

    // std::unique_ptrが自動でglfwDestroyWindowを呼び出す
    glfwTerminate();
}

bool STLViewer::init()
{
    if (!initializeGLFW())
    {
        return false;
    }

    if (!initializeOpenGL())
    {
        return false;
    }

    setupCallbacks();
    return true;
}

bool STLViewer::initializeGLFW()
{
    if (!glfwInit())
    {
        logError("Failed to initialize GLFW", __func__);
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window.reset(glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "STL Viewer", nullptr, nullptr));
    if (!window)
    {
        logError("Failed to create GLFW window", __func__);
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window.get());
    return true;
}

bool STLViewer::initializeOpenGL()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        logError("Failed to initialize GLAD", __func__);
        return false;
    }

    // 深度テストを有効化
    glEnable(GL_DEPTH_TEST);
    return true;
}

void STLViewer::setupCallbacks()
{
    // ウィンドウユーザーポインターを設定
    glfwSetWindowUserPointer(window.get(), this);

    // マウスコールバックを設定
    glfwSetScrollCallback(window.get(), scroll_callback);
}

bool STLViewer::loadSTL(const std::string &filename)
{
    auto loader = ModelLoader{};
    if (!loader.loadFile(filename, mesh))
    {
        auto oss = std::ostringstream{};
        oss << "Failed to load 3D model file: " << loader.getErrorMessage();
        logError(oss.str(), __func__);
        return false;
    }

    // シェーダー設定
    if (!setupShaders())
    {
        return false;
    }

    // 座標軸バッファ設定
    if (!setupAxesBuffers())
    {
        return false;
    }

    // 3Dモデルバッファ設定
    if (!setupModelBuffers())
    {
        return false;
    }

    // カメラ設定
    setupCamera();

    return true;
}

bool STLViewer::setupShaders()
{

    if (!shader.create(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH))
    {
        auto oss = std::ostringstream{};
        oss << "Failed to create shader: " << shader.getErrorMessage();
        logError(oss.str(), __func__);
        return false;
    }

    shader.use();

    return true;
}

bool STLViewer::setupAxesBuffers()
{
    auto vertices = createAxesVertices();
    return createAxesOpenGLBuffers(vertices);
}

bool STLViewer::setupModelBuffers()
{
    auto vertices = convertSTLToVertices();
    return createModelBuffers(vertices);
}

std::vector<float> STLViewer::convertSTLToVertices() const
{
    // 3Dモデルデータを頂点配列に変換（位置3つ + 色3つ + 法線3つ = 9つの値）
    auto vertices = std::vector<float>{};
    vertices.reserve(mesh.triangles.size() * TRIANGLE_VERTICES * VERTEX_COMPONENTS);

    // 各三角形の頂点データをOpenGL用の配列形式に変換
    for (const auto& triangle : mesh.triangles) {
        auto vertexData = triangle.vertices 
            | boost::adaptors::transformed([&triangle](const auto& vertex) {
                return std::array<float, VERTEX_COMPONENTS>{
                    vertex.x, vertex.y, vertex.z,                    // 位置
                    MODEL_COLOR_R, MODEL_COLOR_G, MODEL_COLOR_B,     // 色  
                    triangle.normal.x, triangle.normal.y, triangle.normal.z  // 法線
                };
            });
        
        for (const auto& vertexArray : vertexData) {
            vertices.insert(vertices.end(), vertexArray.begin(), vertexArray.end());
        }
    }

    return vertices;
}

bool STLViewer::createModelBuffers(const std::vector<float> &vertices)
{
    auto buffers = createOpenGLBuffers(vertices);
    modelVAO = buffers.VAO;
    modelVBO = buffers.VBO;
    return true;
}

void STLViewer::setupCamera()
{
    // カメラをさらに遠くに配置
    cameraPos = glm::vec3{CAMERA_DISTANCE, CAMERA_DISTANCE, CAMERA_DISTANCE};
    cameraFront = glm::normalize(glm::vec3{0.0f, 0.0f, 0.0f} - cameraPos); // 原点を向く
    cameraUp = glm::vec3{0.0f, 1.0f, 0.0f};
    
    // ライトの設定
    lightPos = glm::vec3{2.0f, 2.0f, 2.0f};
    lightColor = glm::vec3{1.0f, 1.0f, 1.0f}; // 白色光
}

void STLViewer::run()
{

    while (!glfwWindowShouldClose(window.get()))
    {
        // 入力処理
        processInput();

        // レンダリング
        render();

        // バッファをスワップ
        glfwSwapBuffers(window.get());

        // イベントを処理
        glfwPollEvents();
    }
}

void STLViewer::render()
{
    glClearColor(BACKGROUND_R, BACKGROUND_G, BACKGROUND_B, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();
    updateMatrices();

    renderAxes();
    renderModel();
}

void STLViewer::renderAxes()
{
    // 座標軸を描画（アイデンティティ行列を使用）
    auto axesModel = glm::mat4{1.0f};
    shader.setMat4("model", axesModel);

    glLineWidth(LINE_WIDTH);
    glBindVertexArray(axesVAO);
    glDrawArrays(GL_LINES, 0, AXES_COUNT * AXIS_VERTICES);
    glBindVertexArray(0);
}

void STLViewer::renderModel()
{
    // 3Dモデルを描画（変換されたモデル行列を使用）
    shader.setMat4("model", model);

    glBindVertexArray(modelVAO);
    glDrawArrays(GL_TRIANGLES, 0, mesh.triangles.size() * TRIANGLE_VERTICES);
    glBindVertexArray(0);
}

void STLViewer::updateMatrices()
{
    updateViewProjectionMatrices();
    updateModelMatrix();
    sendMatricesToShader();
}

void STLViewer::updateViewProjectionMatrices()
{
    // ビュー行列
    view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    // プロジェクション行列
    projection =
        glm::perspective(glm::radians(FOV_DEGREES),
                         static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT), NEAR_PLANE, FAR_PLANE);
}

void STLViewer::updateModelMatrix()
{
    // モデル行列（3Dモデルを原点に配置し、適切なサイズにスケール）
    model = glm::mat4{1.0f};

    // 3Dモデルのサイズを計算
    auto objectSize = mesh.max_bounds - mesh.min_bounds;
    auto maxDimension = std::max({objectSize.x, objectSize.y, objectSize.z});

    // 座標軸のサイズ（2.0）に合わせてスケール - もっと大きくする
    auto desiredSize = MODEL_DESIRED_SIZE;
    auto scale = desiredSize / maxDimension;

    // 順序を変更: 1. スケール → 2. 平行移動
    model = glm::scale(model, glm::vec3{scale});
    model = glm::translate(model, -mesh.center * scale);
}

void STLViewer::sendMatricesToShader() const
{
    // シェーダーに行列を送信
    shader.setMat4("model", model);
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    
    // ライティング情報を送信
    shader.setVec3("lightPos", lightPos);
    shader.setVec3("viewPos", cameraPos);
    shader.setVec3("lightColor", lightColor);
    
    // ライティングパラメータを送信
    shader.setFloat("ambientStrength", AMBIENT_STRENGTH);
    shader.setFloat("specularStrength", SPECULAR_STRENGTH);
    shader.setFloat("shininess", SHININESS);
}

void STLViewer::setupVertexAttributes()
{
    // 位置属性
    glVertexAttribPointer(POSITION_ATTRIBUTE_INDEX, POSITION_COMPONENTS, GL_FLOAT, GL_FALSE, 
                          VERTEX_COMPONENTS * sizeof(float), (void *)0);
    glEnableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);

    // 色属性
    glVertexAttribPointer(COLOR_ATTRIBUTE_INDEX, COLOR_COMPONENTS, GL_FLOAT, GL_FALSE, 
                          VERTEX_COMPONENTS * sizeof(float),
                          (void *)(POSITION_COMPONENTS * sizeof(float)));
    glEnableVertexAttribArray(COLOR_ATTRIBUTE_INDEX);

    // 法線属性
    glVertexAttribPointer(NORMAL_ATTRIBUTE_INDEX, NORMAL_COMPONENTS, GL_FLOAT, GL_FALSE, 
                          VERTEX_COMPONENTS * sizeof(float),
                          (void *)((POSITION_COMPONENTS + COLOR_COMPONENTS) * sizeof(float)));
    glEnableVertexAttribArray(NORMAL_ATTRIBUTE_INDEX);
}

std::vector<float> STLViewer::createAxesVertices() const
{
    // 3D座標系の軸を作成（位置3つ + 色3つ + 法線3つ = 9つの値）
    return std::vector<float>{
        // X軸（赤）- 原点から右へ
        0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,        // 原点
        AXIS_LENGTH, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // X軸の先端

        // Y軸（緑）- 原点から上へ
        0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,        // 原点
        0.0f, AXIS_LENGTH, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Y軸の先端

        // Z軸（青）- 原点から手前へ
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,       // 原点
        0.0f, 0.0f, AXIS_LENGTH, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f // Z軸の先端
    };
}

bool STLViewer::createAxesOpenGLBuffers(const std::vector<float>& vertices)
{
    auto buffers = createOpenGLBuffers(vertices);
    axesVAO = buffers.VAO;
    axesVBO = buffers.VBO;
    return true;
}

void STLViewer::processInput()
{
    if (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window.get(), true);
    }
}

STLViewer::BufferPair STLViewer::createOpenGLBuffers(const std::vector<float>& vertices)
{
    BufferPair buffers{};
    
    // VAOとVBOを作成
    glGenVertexArrays(1, &buffers.VAO);
    glGenBuffers(1, &buffers.VBO);

    glBindVertexArray(buffers.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, buffers.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // 頂点属性を設定（共通関数使用）
    setupVertexAttributes();

    glBindVertexArray(0);
    
    return buffers;
}

void STLViewer::logError(const std::string &message, const std::string &functionName) const
{
    if (!functionName.empty())
    {
        std::cerr << "[Error in " << functionName << "] " << message << std::endl;
    }
    else
    {
        std::cerr << "[Error] " << message << std::endl;
    }
}

// グローバルコールバック関数
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    STLViewer *viewer = static_cast<STLViewer *>(glfwGetWindowUserPointer(window));
    if (!viewer)
        return;

    float sensitivity = SCROLL_SENSITIVITY;
    viewer->cameraPos += viewer->cameraFront * static_cast<float>(yoffset) * sensitivity;
}
