#include "shader.h"
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>

// 内部定数定義
namespace
{
constexpr int SHADER_INFO_LOG_SIZE{1024}; // エラーログ用の拡張バッファサイズ
}

Shader::Shader() : programID{0}
{
}

Shader::Shader(Shader &&other) noexcept
    : programID{other.programID}, errorMessage{std::move(other.errorMessage)},
      uniformLocationCache{std::move(other.uniformLocationCache)}
{
    other.programID = 0;
}

Shader &Shader::operator=(Shader &&other) noexcept
{
    if (this != &other)
    {
        cleanup();

        programID = other.programID;
        errorMessage = std::move(other.errorMessage);
        uniformLocationCache = std::move(other.uniformLocationCache);

        other.programID = 0;
    }
    return *this;
}

Shader::~Shader()
{
    cleanup();
}

bool Shader::create(const std::string &vertexPath, const std::string &fragmentPath)
{
    errorMessage.clear();

    // シェーダーファイルを読み込み
    auto vertexCode = loadShaderFile(vertexPath);
    auto fragmentCode = loadShaderFile(fragmentPath);

    if (vertexCode.empty() || fragmentCode.empty())
    {
        return false;
    }

    // シェーダーをコンパイル
    auto vertexShader = compileShader(vertexCode, GL_VERTEX_SHADER);
    auto fragmentShader = compileShader(fragmentCode, GL_FRAGMENT_SHADER);

    if (vertexShader == 0 || fragmentShader == 0)
    {
        return false;
    }

    // シェーダープログラムをリンク
    if (!linkProgram(vertexShader, fragmentShader))
    {
        return false;
    }

    // 個別のシェーダーを削除
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return true;
}

void Shader::use() const
{
    if (programID != 0)
    {
        glUseProgram(programID);
    }
}

void Shader::setBool(const std::string &name, bool value) const
{
    setUniformImpl(name, value, [](GLint location, bool val) {
        glUniform1i(location, static_cast<int>(val));
    });
}

void Shader::setInt(const std::string &name, int value) const
{
    setUniformImpl(name, value, [](GLint location, int val) {
        glUniform1i(location, val);
    });
}

void Shader::setFloat(const std::string &name, float value) const
{
    setUniformImpl(name, value, [](GLint location, float val) {
        glUniform1f(location, val);
    });
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
{
    setUniformImpl(name, value, [](GLint location, const glm::vec3& val) {
        glUniform3fv(location, 1, &val[0]);
    });
}

void Shader::setMat4(const std::string &name, const glm::mat4 &value) const
{
    setUniformImpl(name, value, [](GLint location, const glm::mat4& val) {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(val));
    });
}

std::string Shader::loadShaderFile(const std::string &filePath)
{
    auto file = std::ifstream{filePath};
    if (!file.is_open())
    {
        errorMessage = "Failed to open shader file: " + filePath;
        return "";
    }

    auto buffer = std::stringstream{};
    buffer << file.rdbuf();
    return buffer.str();
}

unsigned int Shader::compileShader(const std::string &source, GLenum type) const
{
    auto shader = glCreateShader(type);
    auto *src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    // コンパイル状態をチェック
    auto success = int{};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        std::vector<char> infoLog(SHADER_INFO_LOG_SIZE);
        glGetShaderInfoLog(shader, SHADER_INFO_LOG_SIZE, nullptr, infoLog.data());

        const char *shaderTypeName = (type == GL_VERTEX_SHADER) ? "Vertex" : "Fragment";
        errorMessage = std::string(shaderTypeName) + " shader compilation failed: " + std::string(infoLog.data());

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool Shader::linkProgram(unsigned int vertexShader, unsigned int fragmentShader)
{
    programID = glCreateProgram();
    glAttachShader(programID, vertexShader);
    glAttachShader(programID, fragmentShader);
    glLinkProgram(programID);

    // リンク状態をチェック
    int success;
    glGetProgramiv(programID, GL_LINK_STATUS, &success);

    if (!success)
    {
        std::vector<char> infoLog(SHADER_INFO_LOG_SIZE);
        glGetProgramInfoLog(programID, SHADER_INFO_LOG_SIZE, nullptr, infoLog.data());
        errorMessage = "Shader program linking failed: " + std::string(infoLog.data());
        return false;
    }

    return true;
}

GLint Shader::getUniformLocation(const std::string &name) const
{
    auto it = uniformLocationCache.find(name);
    if (it != uniformLocationCache.end())
    {
        return it->second;
    }

    auto location = glGetUniformLocation(programID, name.c_str());
    uniformLocationCache[name] = location;
    return location;
}

void Shader::cleanup() noexcept
{
    if (programID != 0)
    {
        glDeleteProgram(programID);
        programID = 0;
    }
    uniformLocationCache.clear();
}