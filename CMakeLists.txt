cmake_minimum_required(VERSION 3.16)
project(STLViewer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vcpkgのツールチェインファイルを指定
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

# vcpkgのパスを設定
set(VCPKG_ROOT "C:/local/vcpkg")
set(CMAKE_TOOLCHAIN_FILE "${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")

# 実行ファイルを追加
add_executable(stl_viewer 
    src/main.cpp
    src/viewer.cpp
    src/model_loader.cpp
    src/shader.cpp
)

# GLFW3を検索
find_package(glfw3 CONFIG REQUIRED)

# GLADを検索
find_package(glad CONFIG REQUIRED)

# Boostを検索
find_package(Boost REQUIRED COMPONENTS program_options)

# GLMを検索
find_package(glm CONFIG REQUIRED)

# Assimpを検索
find_package(assimp CONFIG REQUIRED)

# ライブラリをリンク
target_link_libraries(stl_viewer PRIVATE 
    glfw
    glad::glad
    Boost::program_options
    glm::glm
    assimp::assimp
)

# OpenGLをリンク（Windows）
if(WIN32)
    target_link_libraries(stl_viewer PRIVATE opengl32)
endif()

# C++17 filesystemライブラリをリンク
if(MSVC)
    target_link_libraries(stl_viewer PRIVATE legacy_stdio_definitions)
endif() 