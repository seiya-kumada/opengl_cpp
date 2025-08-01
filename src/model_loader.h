/**
 * @file model_loader.h
 * @brief Assimp を使用した3Dモデル読み込み機能のクラス定義
 * @author STL Viewer Team
 * @version 2.0
 */

#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

// 前方宣言
struct aiScene;
struct aiMesh;
struct aiFace;

namespace Assimp {
    class Importer;
}

/**
 * @brief 3Dモデルの三角形を表す構造体
 * 
 * STLファイルやその他の3Dモデル内の1つの三角形の情報を保持する。
 * 法線ベクトルと3つの頂点座標から構成される。
 */
struct ModelTriangle {
    glm::vec3 normal;      ///< 三角形の法線ベクトル
    glm::vec3 vertices[3]; ///< 三角形の3つの頂点座標
};

/**
 * @brief 3Dモデルメッシュ全体を表す構造体
 * 
 * 3Dモデルファイルから読み込んだ全ての三角形と、
 * 描画に必要な付加情報（バウンディングボックス、中心、スケール）を保持する。
 */
struct ModelMesh {
    std::vector<ModelTriangle> triangles; ///< 三角形データの配列
    
    // 空間情報
    glm::vec3 min_bounds;  ///< バウンディングボックスの最小座標
    glm::vec3 max_bounds;  ///< バウンディングボックスの最大座標
    glm::vec3 center;      ///< メッシュの幾何学的中心
    float scale;           ///< 正規化用のスケール係数
};

/**
 * @brief Assimpを使用した3Dモデルファイルの読み込みを行うクラス
 * 
 * STL、OBJ、FBX、GLTF等の多様な3Dモデル形式に対応し、
 * Assimpライブラリを使用して効率的かつ堅牢な読み込みを提供する。
 * バウンディングボックスの計算や中心座標の算出も自動で行う。
 * 
 * 主な機能:
 * - 50+ 3Dモデル形式の自動判別と読み込み
 * - 三角形メッシュデータの抽出と最適化
 * - バウンディングボックス計算
 * - メッシュの正規化とセンタリング
 * - 詳細なエラーレポート
 * 
 * @note Assimpライブラリが必要
 * @note 大容量ファイル（数十MB）にも対応
 * @note スレッドセーフではない
 */
class ModelLoader {
public:
    /**
     * @brief デフォルトコンストラクタ
     * 
     * エラーメッセージを空文字列で初期化する。
     */
    ModelLoader();
    
    /**
     * @brief デストラクタ
     * 
     * 特別なクリーンアップは不要だが、明示的に定義。
     */
    ~ModelLoader();
    
    // Copy semantics
    /**
     * @brief コピーコンストラクタ
     * 
     * @param other コピー元のModelLoaderオブジェクト
     */
    ModelLoader(const ModelLoader& other);
    
    /**
     * @brief コピー代入演算子
     * 
     * @param other コピー元のModelLoaderオブジェクト
     * @return 自身への参照
     */
    ModelLoader& operator=(const ModelLoader& other);
    
    // Move semantics
    /**
     * @brief ムーブコンストラクタ
     * 
     * @param other ムーブ元のModelLoaderオブジェクト
     */
    ModelLoader(ModelLoader&& other) noexcept;
    
    /**
     * @brief ムーブ代入演算子
     * 
     * @param other ムーブ元のModelLoaderオブジェクト
     * @return 自身への参照
     */
    ModelLoader& operator=(ModelLoader&& other) noexcept;

    /**
     * @brief 3Dモデルファイルを読み込んでメッシュデータを生成する
     * 
     * 指定された3Dモデルファイルをアジンプライブラリで自動判別して読み込み、
     * 三角形メッシュデータとして変換する。同時にバウンディングボックス、
     * 中心座標、スケール係数も計算する。
     * 
     * @param filePath 3Dモデルファイルのパス（相対パス・絶対パス両対応）
     * @param mesh 読み込み結果を格納するModelMeshオブジェクト
     * @return 読み込み成功時はtrue、失敗時はfalse
     * @pre filePathが有効な3Dモデルファイルを指している
     * @post 成功時はmeshに完全なメッシュデータが格納される
     * @post 失敗時はgetErrorMessage()でエラー詳細を取得可能
     */
    bool loadFile(const std::string& filePath, ModelMesh& mesh);
    
    /**
     * @brief 最後に発生したエラーの詳細メッセージを取得する
     * 
     * loadFile()が失敗した場合の詳細な原因を提供する。
     * 
     * @return エラーメッセージ文字列（エラーがない場合は空文字列）
     */
    const std::string& getErrorMessage() const noexcept { return errorMessage; }

private:
    // エラーハンドリング
    mutable std::string errorMessage;  ///< 最後に発生したエラーメッセージ

    /**
     * @brief Assimpシーンからメッシュデータを処理する
     * 
     * @param scene Assimpで読み込んだシーンデータ
     * @param mesh 出力先のメッシュオブジェクト
     * @return 処理成功時はtrue、失敗時はfalse
     */
    bool processScene(const aiScene* scene, ModelMesh& mesh);
    
    /**
     * @brief 単一のAssimpメッシュを処理する
     * 
     * @param aiMesh Assimpメッシュデータ
     * @param mesh 出力先のメッシュオブジェクト
     * @return 処理成功時はtrue、失敗時はfalse
     */
    bool processMesh(const aiMesh* aiMesh, ModelMesh& mesh);
    
    /**
     * @brief メッシュのバウンディングボックスを計算する
     * 
     * 全ての頂点座標から最小・最大座標を求めてmin_bounds、max_boundsに設定する。
     * 
     * @param mesh 計算対象のメッシュオブジェクト
     * @post mesh.min_bounds と mesh.max_bounds が設定される
     */
    void calculateBounds(ModelMesh& mesh);
    
    /**
     * @brief メッシュの中心座標とスケール係数を計算する
     * 
     * バウンディングボックスから幾何学的中心とスケール係数を算出する。
     * 
     * @param mesh 計算対象のメッシュオブジェクト
     * @pre calculateBounds() が既に実行済みであること
     * @post mesh.center と mesh.scale が設定される
     */
    void calculateCenterAndScale(ModelMesh& mesh);
    
    /**
     * @brief Assimpインポーターを設定し、ファイルを読み込む
     * 
     * @param filePath 読み込むファイルのパス
     * @return 読み込んだシーンポインタ（失敗時はnullptr）
     */
    const aiScene* loadFileWithAssimp(const std::string& filePath, Assimp::Importer& importer);
    
    /**
     * @brief 読み込んだシーンの基本検証を行う
     * 
     * @param scene 検証するシーン
     * @return 検証成功時はtrue、失敗時はfalse
     */
    bool validateScene(const aiScene* scene);
    
    /**
     * @brief メッシュ処理結果の検証を行う
     * 
     * @param mesh 検証するメッシュ
     * @return 検証成功時はtrue、失敗時はfalse
     */
    bool validateProcessedMesh(const ModelMesh& mesh);
    
    /**
     * @brief 単一の三角形を作成する
     * 
     * @param face Assimpの面データ
     * @param aiMesh Assimpメッシュデータ
     * @param triangle 出力先の三角形オブジェクト
     * @return 作成成功時はtrue、失敗時はfalse
     */
    bool createTriangle(const aiFace& face, const aiMesh* aiMesh, ModelTriangle& triangle);
    
    /**
     * @brief エラーメッセージを内部的に設定する
     * 
     * @param message エラーメッセージ
     * @param context エラーのコンテキスト（省略可）
     */
    void setError(const std::string& message, const std::string& context = "");
};