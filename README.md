# OpenGL STL ビューアー

Windows上で動作するOpenGLを用いた3Dモデルビューアーアプリケーションです。STLファイルを含む50種類以上の3Dモデル形式に対応し、Claude DesktopのMCPサーバーとしても利用できます。

## 🎯 特徴

- **多形式対応**: STL、OBJ、FBX、GLTF等50+の3Dモデル形式
- **高品質レンダリング**: OpenGL 3.3 Core Profile + Phongシェーディング
- **MCPサーバー統合**: Claude Desktopから直接3Dモデルを表示
- **軽量・高速**: VBOによる最適化、100万面まで対応

## 🎮 操作方法

- **マウスホイール**: ズームイン/アウト
- **ESCキー**: ビューアー終了

## 🚀 クイックスタート

### 必要環境
- Windows 10/11
- OpenGL 3.3対応GPU
- Visual Studio Community 2022

### ビルド手順

1. **依存関係のインストール**
```powershell
# vcpkgで必要ライブラリをインストール
C:\local\vcpkg\vcpkg.exe install glfw3:x64-windows
C:\local\vcpkg\vcpkg.exe install glad:x64-windows
C:\local\vcpkg\vcpkg.exe install glm:x64-windows
C:\local\vcpkg\vcpkg.exe install assimp:x64-windows
C:\local\vcpkg\vcpkg.exe install boost:x64-windows
```

2. **プロジェクトビルド**
```powershell
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build . --config Release
```

3. **実行**
```powershell
cd Release
.\stl_viewer.exe path/to/model.stl
```

## 🤖 Claude Desktop MCP サーバー

Claude Desktopから3Dモデルを直接表示できます。

### セットアップ

1. **Python環境構築**
```powershell
cd mcp-server
python -m venv .venv
.venv\Scripts\activate
pip install -e .
```

2. **Claude Desktop設定**
`%APPDATA%\Claude\claude_desktop_config.json` に追加：
```json
{
  "mcpServers": {
    "stl-viewer": {
      "command": "C:/projects/opengl_cpp/mcp-server/.venv/Scripts/python.exe",
      "args": ["C:/projects/opengl_cpp/mcp-server/src/stl_viewer_server.py"],
      "cwd": "C:/projects/opengl_cpp/mcp-server"
    }
  }
}
```

3. **使用方法**
Claude Desktopで「3Dモデルを表示してください: C:/path/to/model.stl」と入力

## 🏗️ アーキテクチャ

### プロジェクト構造
```
opengl_cpp/
├── src/
│   ├── main.cpp          # エントリーポイント
│   ├── viewer.cpp/h      # メインビューアークラス
│   ├── model_loader.cpp/h # Assimp 3Dモデル読み込み
│   └── shader.cpp/h      # シェーダー管理
├── shaders/
│   ├── vertex.glsl       # 頂点シェーダー
│   └── fragment.glsl     # フラグメントシェーダー
├── mcp-server/           # Claude Desktop MCP サーバー
└── stls/                 # サンプルファイル
```

### 技術スタック
- **レンダリング**: OpenGL 3.3 Core Profile
- **ウィンドウ管理**: GLFW
- **数学ライブラリ**: GLM
- **3Dモデル読み込み**: Assimp
- **モダンC++**: C++17 + boost/ranges
- **MCPサーバー**: Python 3.10+

## 🔧 開発者向け情報

### OpenGL初期化順序
```
GLFW → GLAD → OpenGL
```

### レンダリングパイプライン
```
C++頂点データ → 頂点シェーダー → ラスタライズ → フラグメントシェーダー → 画面
```

### 実装済み機能
- ✅ Phongライティング（環境光・拡散反射・鏡面反射）
- ✅ 自動モデルセンタリング・スケーリング
- ✅ 3D座標軸表示
- ✅ マウススクロールズーム

### 今後の拡張予定
- マウスドラッグによる回転・移動

---

**プロジェクト作成者**: seiya-kumada

**リポジトリ**: https://github.com/seiya-kumada/opengl_cpp