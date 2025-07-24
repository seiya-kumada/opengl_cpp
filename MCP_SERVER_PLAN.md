# STL Viewer MCP Server 作成計画

## 1. プロジェクト構造設計
```
C:\projects\opengl_cpp\
├── src/              # 既存のC++コード
├── build/            # 既存のビルドディレクトリ  
├── shaders/          # 既存のシェーダー
└── mcp-server/       # 新規：MCP サーバー専用ディレクトリ
    ├── src/
    │   └── stl_viewer_server.py    # メインサーバーコード
    ├── requirements.txt            # Python依存関係
    ├── pyproject.toml             # プロジェクト設定
    └── README.md                  # MCP サーバー用ドキュメント
```

## 2. MCP ツール設計
- **ツール名**: `display_3d_model`
- **パラメータ**: 
  - `file_path` (required): STL/3Dモデルファイルのパス
  - `window_width` (optional): ウィンドウ幅 (デフォルト: 800)
  - `window_height` (optional): ウィンドウ高 (デフォルト: 600)

## 3. 実装手順
1. **MCP サーバー環境セットアップ**
   - mcp-server/ディレクトリ作成
   - Python仮想環境構築
   - MCP SDK インストール

2. **MCP サーバー実装**
   - Python MCP サーバーコード作成
   - C++ STL Viewer へのプロセス呼び出し実装
   - エラーハンドリング追加

3. **Claude Desktop 連携設定**
   - MCP サーバー設定ファイル作成
   - Claude Desktop 設定に追加

4. **テスト・検証**
   - サンプルSTLファイルでの動作確認
   - Claude Desktop からの呼び出しテスト

## 4. 技術仕様
- **言語**: Python 3.10+
- **MCP SDK**: modelcontextprotocol/python-sdk
- **C++ 連携**: subprocess でstl_viewer.exe呼び出し
- **対応形式**: STL (ASCII/Binary) + Assimp対応全形式

## 5. Claude Desktop 設定

Claude Desktop の設定ファイル (`%APPDATA%\Claude\claude_desktop_config.json`) に以下を追加：

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

## 6. 使用方法

Claude Desktop で以下のように呼び出し可能：

```
Please display the 3D model at C:/path/to/model.stl
```

ツールパラメータ：
- **file_path** (必須): 3Dモデルファイルのパス
- **window_width** (任意): ウィンドウ幅（デフォルト: 800）
- **window_height** (任意): ウィンドウ高（デフォルト: 600）