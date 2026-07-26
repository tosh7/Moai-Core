# Moai-Core
Moai フレームワークのコアアルゴリズム。

[![Test](https://github.com/tosh7/Moai-Core/actions/workflows/test.yml/badge.svg)](https://github.com/tosh7/Moai-Core/actions/workflows/test.yml)

[English](README.md)

## 動作環境
Version: C++23

## Elevator

待っているコールから行き先を判断する、単機のエレベーターです。

コア側はタイマーもスレッドも持ちません。`step()` が呼ばれるまで何も動かないので、
時間の進め方は呼び出し側が握ります。`Timer` から回しても、`CADisplayLink` に
合わせても、テストで一気に回しても構いません。

```cpp
#include "elevator.h"

Elevator elevator(10, 1);                 // 1階から10階まで。1階で待機

elevator.request({5, Direction::UP});     // 5階で上りボタンが押された

elevator.step();                          // 1回の呼び出しで1フロア進む
elevator.current_floor;                   // 2
```

`request` はホールコール、つまり「何階でボタンが押されたか」と「その人がどちらへ
行きたいか」を記録します。同じ階の上りと下りは別のコールとして扱われます。建物の
範囲外は無視され、同じボタンを二度押しても何も変わりません。

`step` はエレベーターを1フロア進めます。進行方向にコールが残っている限りその向きを
保ち、無くなったら反転します。そのため1階から5階へ向かう途中に3階のコールが入れば、
引き返さずに通り道で拾います。乗せるのは進行方向と一致するコールだけですが、
折り返す階では例外で、そこから先に用が無い以上、逆方向のコールもそこで乗せます。

未対応: かご内ボタン（カーコール）、ドアの開閉状態、地下階、複数台の配車。

## ビルド方法
.a ファイルを作るには以下を実行します。
1. このリポジトリを clone する
2. `sh build.sh` を実行する

build ディレクトリに .a ファイルが出力されます。

このリポジトリは Swift package でもあるため、Xcode や他の `Package.swift` から
そのまま依存に追加できます。

## テスト方法
`sh test.sh` を実行します。ソースと `tests/` をまとめてビルドして実行し、
何件のチェックが通ったかを表示します。失敗があれば非ゼロで終了します。

## ライセンス
[MIT License](https://github.com/tosh7/Moai-Core/blob/main/LICENSE)

## 連絡先
- e-mail: zlia.6.lj.425@gmail.com
- Twitter: [tosh_3](https://x.com/tosh_3)
- Linked In: [Satoshi Komatsu](https://www.linkedin.com/in/satoshi-komatsu-5a8a4a220/)
