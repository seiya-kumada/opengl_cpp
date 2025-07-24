# Visual Studio Community + CMake + vcpkg ビルドガイド

## 概要
このガイドは、CMakeとvcpkgを使ってVisual Studio CommunityでC++プロジェクトをビルドする手順をまとめたものです。

---

## 1. 必要なツールのインストール

- **Visual Studio Community 2022**（C++デスクトップ開発ワークロード）
- **CMake**（公式サイトまたはchoco/scoopでインストール）
- **vcpkg**（https://github.com/microsoft/vcpkg）

---

## 2. vcpkgで依存ライブラリをインストール

```powershell
# 例: GLFWとGLADをインストール
C:\local\vcpkg\vcpkg.exe install glfw3:x64-windows
C:\local\vcpkg\vcpkg.exe install glad:x64-windows
```

---

## 3. CMakeプロジェクトのビルド

```powershell
cd プロジェクトルート
mkdir build
cd build

cmake -DCMAKE_TOOLCHAIN_FILE=C:/local/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build .
```

---

## 4. 実行

```powershell
cd Debug
.\stl_viewer.exe dummy.stl
```

---

## 補足
- 旧バッチファイル手順は不要です。
- vcpkgのパスやCMakeのバージョンは環境に合わせて適宜修正してください。
- 依存ライブラリを追加したい場合はvcpkgでインストールし、CMakeLists.txtに`find_package`と`target_link_libraries`を追加してください。 