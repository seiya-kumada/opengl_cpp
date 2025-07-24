# GLFW vs GLAD vs GL
GLFW -> GLAD -> GLの順で初期化する

## GLFW（OpenGL Framework）
役割：
- ウィンドウや入力の管理

例：
- ウィンドウの作成、マウス処理

## GLAD（OpenGL Loader Generator）
役割：
- OpenGL関数の読み込み。

例：
- 関数ポインタの取得

## GL（OpenGL）
役割：
- 3Dグラフィックス描画

例：
- 実際の描画命令

# シェーダーの役割
描画パイプラインの流れは以下の通り
1. C++頂点データ
1. 頂点シェーダー(vertex.glsl)
1. ラスタライズ
1. フラグメントシェーダー(fragment.glsl)
1. 画面

## 三角形を描く場合
- 頂点シェーダー：3つの頂点に対し実行される。
- フラグメントシェーダー：三角形内部の全ピクセルに対し実行される。


STLViewer::loadSTL内のsetupAxesBuffersから