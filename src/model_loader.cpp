#include "model_loader.h"
#include <algorithm>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/irange.hpp>
#include <boost/range/numeric.hpp>
#include <iostream>

// 内部定数定義
namespace
{
constexpr int TRIANGLE_VERTICES{3};              // 三角形の頂点数
constexpr float CENTER_CALCULATION_FACTOR{0.5f}; // 中心計算用係数
constexpr float DEFAULT_SCALE{1.0f};             // デフォルトスケール
constexpr float EPSILON{1e-6f};                  // 浮動小数点ゼロ判定用イプシロン
} // namespace

ModelLoader::ModelLoader()
{
}

ModelLoader::~ModelLoader()
{
}

// Copy semantics
ModelLoader::ModelLoader(const ModelLoader &other) : errorMessage(other.errorMessage)
{
}

ModelLoader &ModelLoader::operator=(const ModelLoader &other)
{
    if (this != &other)
    {
        errorMessage = other.errorMessage;
    }
    return *this;
}

// Move semantics
ModelLoader::ModelLoader(ModelLoader &&other) noexcept : errorMessage(std::move(other.errorMessage))
{
}

ModelLoader &ModelLoader::operator=(ModelLoader &&other) noexcept
{
    if (this != &other)
    {
        errorMessage = std::move(other.errorMessage);
    }
    return *this;
}

bool ModelLoader::loadFile(const std::string &filePath, ModelMesh &mesh)
{
    errorMessage.clear();

    // Assimpインポーターを作成し、ファイルを読み込み
    auto importer = Assimp::Importer{};
    auto scene = loadFileWithAssimp(filePath, importer);
    if (!scene)
    {
        return false;
    }

    // シーンの基本検証
    if (!validateScene(scene))
    {
        return false;
    }

    // シーンからメッシュデータを処理
    if (!processScene(scene, mesh))
    {
        return false;
    }

    // 処理結果の検証
    if (!validateProcessedMesh(mesh))
    {
        return false;
    }

    // バウンディングボックスと中心・スケールを計算
    calculateBounds(mesh);
    calculateCenterAndScale(mesh);

    return true;
}

const aiScene* ModelLoader::loadFileWithAssimp(const std::string& filePath, Assimp::Importer& importer)
{
    // ファイルを読み込み（自動で最適化処理を適用）
    auto flags = aiProcess_Triangulate |           // 全てのポリゴンを三角形に変換
                 aiProcess_GenNormals |            // 法線ベクトルを自動生成
                 aiProcess_ValidateDataStructure | // データ構造の妥当性を検証
                 aiProcess_JoinIdenticalVertices | // 重複頂点を統合
                 aiProcess_SortByPType |           // プリミティブタイプでソート
                 aiProcess_OptimizeMeshes;         // メッシュを最適化
    
    auto scene = importer.ReadFile(filePath, flags);
    if (!scene)
    {
        setError("Failed to load 3D model", importer.GetErrorString());
        return nullptr;
    }
    
    return scene;
}

bool ModelLoader::validateScene(const aiScene* scene)
{
    if (!scene->HasMeshes())
    {
        setError("No mesh data found in the file", "File might be empty or corrupted");
        return false;
    }
    return true;
}

bool ModelLoader::validateProcessedMesh(const ModelMesh& mesh)
{
    if (mesh.triangles.empty())
    {
        setError("No triangle data could be extracted from the file",
                 "Model might contain only points/lines or unsupported geometry");
        return false;
    }
    return true;
}

bool ModelLoader::processScene(const aiScene *scene, ModelMesh &mesh)
{
    // メモリ効率化: 総三角形数を推定してreserve（boost/ranges版）
    auto meshIndices = boost::irange(0u, scene->mNumMeshes);
    auto estimatedTriangles = boost::accumulate(meshIndices | boost::adaptors::transformed([scene](unsigned int i) {
                                                    return static_cast<size_t>(scene->mMeshes[i]->mNumFaces);
                                                }),
                                                size_t{0});
    mesh.triangles.reserve(estimatedTriangles);

    // 全てのメッシュを処理（通常、STLファイルは1つのメッシュを持つ）
    return std::all_of(meshIndices.begin(), meshIndices.end(),
                       [this, scene, &mesh](unsigned int i) { return processMesh(scene->mMeshes[i], mesh); });
}

bool ModelLoader::processMesh(const aiMesh *aiMesh, ModelMesh &mesh)
{
    if (!aiMesh->HasFaces())
    {
        setError("Mesh does not contain face data", "Mesh index: " + std::to_string(mesh.triangles.size() / 1000));
        return false;
    }

    if (!aiMesh->HasPositions())
    {
        setError("Mesh does not contain vertex position data",
                 "Mesh index: " + std::to_string(mesh.triangles.size() / 1000));
        return false;
    }

    // 既存の三角形数を記録（複数メッシュの場合に備えて）
    size_t initialTriangleCount = mesh.triangles.size();

    // 三角形データを抽出
    for (unsigned int i = 0; i < aiMesh->mNumFaces; ++i)
    {
        const aiFace &face = aiMesh->mFaces[i];

        // AssimpのaiProcess_Triangulateにより、全ての面は三角形になっている
        if (face.mNumIndices == TRIANGLE_VERTICES)
        {
            auto triangle = ModelTriangle{};

            if (!createTriangle(face, aiMesh, triangle))
            {
                return false;
            }

            mesh.triangles.push_back(triangle);
        }
    }

    // 処理された三角形数をログ出力（デバッグ用、必要に応じてコメントアウト）
    // std::cout << "Processed " << (mesh.triangles.size() - initialTriangleCount)
    //           << " triangles from mesh" << std::endl;

    return true;
}

void ModelLoader::calculateBounds(ModelMesh &mesh)
{
    if (mesh.triangles.empty())
    {
        return;
    }

    // 最初の三角形で初期化
    mesh.min_bounds = mesh.max_bounds = mesh.triangles[0].vertices[0];

    // すべての頂点をチェックしてバウンディングボックスを計算（最適化版）
    for (const auto &triangle : mesh.triangles)
    {
        for (int i = 0; i < TRIANGLE_VERTICES; ++i)
        {
            const glm::vec3 &vertex = triangle.vertices[i];

            // 各軸で最小・最大を同時に更新（分岐予測最適化）
            mesh.min_bounds.x = std::min(mesh.min_bounds.x, vertex.x);
            mesh.max_bounds.x = std::max(mesh.max_bounds.x, vertex.x);

            mesh.min_bounds.y = std::min(mesh.min_bounds.y, vertex.y);
            mesh.max_bounds.y = std::max(mesh.max_bounds.y, vertex.y);

            mesh.min_bounds.z = std::min(mesh.min_bounds.z, vertex.z);
            mesh.max_bounds.z = std::max(mesh.max_bounds.z, vertex.z);
        }
    }
}

void ModelLoader::calculateCenterAndScale(ModelMesh &mesh)
{
    if (mesh.triangles.empty())
    {
        return;
    }

    // 中心を計算
    mesh.center = (mesh.min_bounds + mesh.max_bounds) * CENTER_CALCULATION_FACTOR;

    // スケールを計算（最大の軸の長さで正規化）
    auto size = mesh.max_bounds - mesh.min_bounds;
    auto maxSize = std::max({size.x, size.y, size.z});

    if (maxSize > EPSILON)
    {
        // スケールを調整してモデルが見やすいサイズになるように
        mesh.scale = DEFAULT_SCALE / maxSize;
    }
    else
    {
        mesh.scale = DEFAULT_SCALE;
    }
}

bool ModelLoader::createTriangle(const aiFace &face, const aiMesh *aiMesh, ModelTriangle &triangle)
{
    // 3つの頂点を取得
    for (int j = 0; j < TRIANGLE_VERTICES; ++j)
    {
        if (face.mIndices[j] >= aiMesh->mNumVertices)
        {
            setError("Invalid vertex index in face data", "Index: " + std::to_string(face.mIndices[j]));
            return false;
        }

        const auto &vertex = aiMesh->mVertices[face.mIndices[j]];
        triangle.vertices[j] = glm::vec3{vertex.x, vertex.y, vertex.z};
    }

    // 法線ベクトルを取得（利用可能な場合）
    if (aiMesh->HasNormals())
    {
        const auto &normal = aiMesh->mNormals[face.mIndices[0]];
        triangle.normal = glm::vec3{normal.x, normal.y, normal.z};
    }
    else
    {
        // 法線が無い場合は面の法線を計算
        auto v1 = triangle.vertices[1] - triangle.vertices[0];
        auto v2 = triangle.vertices[2] - triangle.vertices[0];
        triangle.normal = glm::normalize(glm::cross(v1, v2));
    }

    return true;
}

void ModelLoader::setError(const std::string &message, const std::string &context)
{
    if (!context.empty())
    {
        errorMessage = message + ": " + context;
    }
    else
    {
        errorMessage = message;
    }
}