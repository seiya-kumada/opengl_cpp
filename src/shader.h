/**
 * @file shader.h
 * @brief OpenGL シェーダープログラム管理クラスの定義
 * @author STL Viewer Team
 * @version 1.0
 */

#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

/**
 * @brief OpenGL シェーダープログラムを管理するクラス
 * 
 * このクラスは頂点シェーダーとフラグメントシェーダーのコンパイル、リンク、
 * および uniform 変数の設定を行う。パフォーマンス向上のため uniform location の
 * キャッシュ機能も提供する。
 * 
 * 主な機能:
 * - 頂点・フラグメントシェーダーファイルの読み込み
 * - シェーダーのコンパイルとプログラムリンク
 * - uniform 変数の効率的な設定（location キャッシュ付き）
 * - 包括的なエラーハンドリング
 * 
 * @note OpenGL 3.3 Core Profile 対応
 * @note RAII パターンによるリソース管理
 * @note 移動セマンティクス対応
 */
class Shader {
public:
    /**
     * @brief デフォルトコンストラクタ
     * 
     * シェーダープログラム ID を無効値で初期化する。
     */
    Shader();
    
    /**
     * @brief 移動コンストラクタ
     * 
     * 他のShaderオブジェクトからリソースを移動する。
     * 
     * @param other 移動元のShaderオブジェクト
     */
    Shader(Shader&& other) noexcept;
    
    /**
     * @brief 移動代入演算子
     * 
     * 他のShaderオブジェクトからリソースを移動代入する。
     * 
     * @param other 移動元のShaderオブジェクト
     * @return 自身への参照
     */
    Shader& operator=(Shader&& other) noexcept;
    
    /**
     * @brief デストラクタ
     * 
     * OpenGL シェーダープログラムリソースを解放する。
     */
    ~Shader();
    
    // コピーコンストラクタと代入演算子を削除（リソース管理のため）
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    
    /**
     * @brief シェーダープログラムを作成する
     * 
     * 指定されたファイルパスから頂点シェーダーとフラグメントシェーダーを
     * 読み込み、コンパイル、リンクしてシェーダープログラムを作成する。
     * 
     * @param vertexPath 頂点シェーダーファイルのパス
     * @param fragmentPath フラグメントシェーダーファイルのパス
     * @return 作成成功時は true、失敗時は false
     * @pre vertexPath と fragmentPath が有効なシェーダーファイルを指している
     * @post 成功時はシェーダープログラムが使用可能になる
     */
    bool create(const std::string& vertexPath, const std::string& fragmentPath);
    
    /**
     * @brief シェーダープログラムをアクティブにする
     * 
     * このシェーダープログラムを現在の描画で使用するように設定する。
     * 
     * @pre create() が正常に完了している
     */
    void use() const;
    
    /**
     * @brief bool 型の uniform 変数を設定する
     * 
     * @param name uniform 変数名
     * @param value 設定する値
     */
    void setBool(const std::string& name, bool value) const;
    
    /**
     * @brief int 型の uniform 変数を設定する
     * 
     * @param name uniform 変数名
     * @param value 設定する値
     */
    void setInt(const std::string& name, int value) const;
    
    /**
     * @brief float 型の uniform 変数を設定する
     * 
     * @param name uniform 変数名
     * @param value 設定する値
     */
    void setFloat(const std::string& name, float value) const;
    
    /**
     * @brief vec3 型の uniform 変数を設定する
     * 
     * @param name uniform 変数名
     * @param value 設定する値
     */
    void setVec3(const std::string& name, const glm::vec3& value) const;
    
    /**
     * @brief mat4 型の uniform 変数を設定する
     * 
     * @param name uniform 変数名
     * @param value 設定する値
     */
    void setMat4(const std::string& name, const glm::mat4& value) const;
    
    /**
     * @brief 最後に発生したエラーメッセージを取得する
     * 
     * @return エラーメッセージ文字列
     */
    const std::string& getErrorMessage() const noexcept { return errorMessage; }
    
    /**
     * @brief シェーダープログラムが有効かどうかを確認する
     * 
     * @return 有効な場合は true、無効な場合は false
     */
    bool isValid() const noexcept { return programID != 0; }

private:
    // OpenGL リソース
    unsigned int programID;                           ///< シェーダープログラム ID
    
    // エラーハンドリング
    mutable std::string errorMessage;                 ///< 最後に発生したエラーメッセージ
    
    // パフォーマンス最適化
    mutable std::unordered_map<std::string, GLint> uniformLocationCache;  ///< uniform location キャッシュ
    
    /**
     * @brief シェーダーファイルをテキストとして読み込む
     * 
     * @param filePath ファイルパス
     * @return ファイル内容（失敗時は空文字列）
     */
    std::string loadShaderFile(const std::string& filePath);
    
    /**
     * @brief シェーダーソースをコンパイルする
     * 
     * @param source シェーダーソースコード
     * @param type シェーダータイプ（GL_VERTEX_SHADER など）
     * @return コンパイル済みシェーダー ID（失敗時は 0）
     */
    unsigned int compileShader(const std::string& source, GLenum type) const;
    
    /**
     * @brief シェーダープログラムをリンクする
     * 
     * @param vertexShader 頂点シェーダー ID
     * @param fragmentShader フラグメントシェーダー ID
     * @return リンク成功時は true、失敗時は false
     */
    bool linkProgram(unsigned int vertexShader, unsigned int fragmentShader);
    
    /**
     * @brief uniform 変数の location を取得する（キャッシュ付き）
     * 
     * @param name uniform 変数名
     * @return uniform location（見つからない場合は -1）
     */
    GLint getUniformLocation(const std::string& name) const;
    
    /**
     * @brief OpenGL リソースを解放する
     */
    void cleanup() noexcept;
}; 