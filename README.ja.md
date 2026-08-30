# Moai-Core
Moai フレームワークのコアアルゴリズム。

[![Test](https://github.com/tosh7/Moai-Core/actions/workflows/test.yml/badge.svg)](https://github.com/tosh7/Moai-Core/actions/workflows/test.yml)

[English](README.md)

## 動作環境
Version: C++23

ビルドには CMake 3.20 以降が必要です。Ninja があればそれを、無ければ make を
使うので、どちらも事前に用意しなくて構いません。

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

## World

落ちて、ぶつかって、積み上がる円たちの入れ物です。

エレベーターと同じく時計を持ちません。`step(dt)` が決まった幅だけ時間を進め、
それを何回呼ぶかは呼び出し側が決めます。画面では見やすい速さで、テストでは一瞬で、
同じ動きを再現できます。刻み幅が固定なので結果も毎回同じで、同じ位置から落とせば
同じように積み上がります。

```cpp
#include "world.h"

World world(400, 800);                       // 400×800 の空間。床は y = 0
world.set_gravity({0, -1000});               // y は上向きなので下は負

int body = world.add_body({200, 700}, 20);   // 半径 20 の円

world.step(1.0f / 60);                       // 1 コマぶん時間を進める
world.position_of(body);                     // 今どこに描けばよいか
```

`add_body` はポインタではなく添字を返します。物体が増えても掴んだ相手を見失いません。
扱えるのは円だけで、他の形も回転もありません。

`step` は重力を積分し、重なった物体を押し離し、ぶつかった勢いを跳ね返し、すべてを
壁の内側に留めます。重力で沈んで触れただけの組は跳ねず、本当にぶつかったときだけ
跳ねます。

`apply_radial_impulse` は、ある点の周囲にあるものを外向きに突き飛ばします。近いほど
強く飛ぶので、積み上がった山を叩いて散らすような動きになります。

未対応: 円以外の形、回転、物体ごとの質量や材質、ジョイント。

## ビルド方法
.a ファイルを作るには以下を実行します。
1. このリポジトリを clone する
2. `sh build.sh` を実行する

`build/ios/libmoai.a` が出力されます。

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
