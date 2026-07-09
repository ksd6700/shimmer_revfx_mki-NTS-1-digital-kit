# ShimmerVerb for KORG NTS-1 mkI

日本語は上、English is below.

ShimmerVerb は KORG Nu:Tekt NTS-1 digital kit mkI 用のカスタム・リバーブ・エフェクトです。通常の残響に、1オクターブ上へピッチシフトした成分をフィードバックして、明るく浮遊感のあるシマーリバーブを作ります。

## どのファイルをダウンロードすればいい？

| 目的 | ダウンロードするもの |
| --- | --- |
| NTS-1 に入れてすぐ使いたい | GitHub Releases に `shimmer_revfx.ntkdigunit` がある場合は、それをダウンロードしてください。これが NTS-1 Sound Librarian に読み込ませる完成ファイルです。 |
| Releases に `.ntkdigunit` がない | このリポジトリはソースコード版です。`Code` -> `Download ZIP` でリポジトリ全体をダウンロードするか、Git で clone して、logue SDK でビルドしてください。 |
| 改造したい、DSPコードを読みたい | リポジトリ全体を clone してください。`shimmer_revfx.cpp` だけではビルドできません。 |

NTS-1 Sound Librarian に読み込ませるファイルは `.ntkdigunit` です。`shimmer_revfx.cpp`、`manifest.json`、`Makefile`、`ld/`、`tpl/` はビルド用のソース／設定ファイルです。

## 対応機種

- KORG Nu:Tekt NTS-1 digital kit mkI
- logue SDK `nutekt-digital`
- API `1.1-0`
- NTS-1 mkII ではなく、初代 NTS-1 mkI 向けです

KORG の logue SDK README では、SDK `1.1-0` で作られた user unit には NTS-1 firmware `1.02` 以降が必要とされています。

## インストール方法

完成済みの `shimmer_revfx.ntkdigunit` がある場合:

1. KORG NTS-1 Sound Librarian をインストールします。
2. NTS-1 を USB でコンピューターに接続します。
3. Sound Librarian で `shimmer_revfx.ntkdigunit` を USER REVERB FX 側へ読み込みます。
4. `SEND ALL` または該当操作で NTS-1 に転送します。
5. NTS-1 の reverb タイプから `ShimmerVerb` を選びます。

KORG の Sound Librarian では、custom oscillator / effect は user unit ファイルとして配布され、対応するリストへドラッグ＆ドロップして NTS-1 に転送できます。

## 操作

| NTS-1 の操作子 | 内容 |
| --- | --- |
| `TIME` | リバーブの残響時間 |
| `DEPTH` | ドライ音とウェット音のミックス |
| `SHIFT + DEPTH` | シマー量。1オクターブ上の成分をどれだけ残響へ戻すか |

おすすめの開始点:

- `TIME`: 50-70%
- `DEPTH`: 30-45%
- `SHIFT + DEPTH`: 25-50%

シマー量を上げると、尾が明るく伸びてパッドのようになります。響きが膨らみすぎる、または高域が目立ちすぎる場合は、まず `SHIFT + DEPTH` を下げてください。

## このエフェクトの背景

シマーリバーブは、リバーブの残響音にピッチシフトを組み合わせたエフェクトです。典型的には、残響の一部を1オクターブ上に上げ、それをもう一度リバーブへ戻します。すると、普通のホールリバーブよりも明るく、上方向へ伸びるような、少し非現実的な尾ができます。

ギター、シンセのコード、アルペジオ、外部入力のドローンに向いています。NTS-1 は小さな機材ですが、外部入力にもエフェクトをかけられるので、卓上のシマー空間として使うとかなり楽しいです。

この ShimmerVerb は、巨大なスタジオ・リバーブをそのまま再現するよりも、NTS-1 の小さなCPUとメモリの中で気持ちよく鳴ることを優先しています。軽く、少しざらっとしていて、演奏中にノブを動かしても破綻しにくいキャラクターを狙っています。

## 実装の中身

主なDSP構成:

- 4本の feedback delay network によるリバーブ本体
- 2段の allpass diffuser による入力拡散
- 2タップの moving delay による簡易オクターブアップ・ピッチシフタ
- 低域が暴れないようにした high-passed shimmer feedback
- 出力とフィードバック部の soft clip による過大入力対策
- 大きなディレイバッファは `__sdram` に配置

`TIME` は feedback decay と damping を同時に動かします。`DEPTH` はウェットミックスです。`SHIFT + DEPTH` はピッチシフトした成分のフィードバック量で、上げるほど「天井が開く」ような響きになります。

このピッチシフタは高品位な独立ピッチシフト・アルゴリズムではなく、シマーリバーブのフィードバック用に軽量化したものです。そのため、設定によっては少し揺れや粒立ちがあります。それもこのユニットの質感として扱っています。

## ソースからビルドする

KORG の logue SDK と Arm toolchain が必要です。

まず logue SDK を用意します。

```sh
git clone https://github.com/korginc/logue-sdk.git
cd logue-sdk
git submodule update --init
```

このリポジトリを `platform/nutekt-digital` の下に clone します。

```sh
cd platform/nutekt-digital
git clone https://github.com/ksd6700/shimmer_revfx_mki-NTS-1-digital-kit.git shimmer-revfx
cd shimmer-revfx
make install
```

別の場所に clone している場合は、`PLATFORMDIR` を指定してビルドできます。

```sh
make PLATFORMDIR=/path/to/logue-sdk/platform/nutekt-digital install
```

成功すると、次のファイルができます。

```text
shimmer_revfx.ntkdigunit
```

これを NTS-1 Sound Librarian で読み込んでください。

## ファイル構成

| ファイル / フォルダ | 内容 |
| --- | --- |
| `shimmer_revfx.cpp` | エフェクト本体のDSP実装 |
| `manifest.json` | NTS-1 user unit のメタデータ |
| `Makefile`, `project.mk` | logue SDK 用ビルド設定 |
| `ld/` | reverb effect 用リンカ設定 |
| `tpl/` | user unit entry template |
| `test-stubs/` | デスクトップで構文チェックするための最小スタブ。実機ビルドでは使いません。 |

## 開発メモ

ローカルの簡易構文チェック:

```sh
g++ -std=c++11 -Wall -Wextra -I test-stubs -fsyntax-only shimmer_revfx.cpp
python3 -m json.tool manifest.json
```

これは実機向けバイナリを作るチェックではありません。最終的な `.ntkdigunit` は KORG logue SDK の toolchain でビルドしてください。

## 参考リンク

- KORG NTS-1 Sound Librarian: https://www.korg.com/us/products/dj/nts_1/librarian_contents.php
- KORG logue SDK: https://github.com/korginc/logue-sdk
- NTS-1 custom effects article: https://www.korg.com/us/products/dj/nts_1/custom_effects.php

## ライセンス

KORG template files are covered by `LICENSE-KORG-BSD-3-Clause.txt`.

---

# ShimmerVerb for KORG NTS-1 mkI

ShimmerVerb is a custom reverb effect for the KORG Nu:Tekt NTS-1 digital kit mkI. It adds an octave-up pitch-shifted signal into the reverb feedback path, creating a bright, floating shimmer tail.

## Which file should I download?

| Goal | What to download |
| --- | --- |
| I just want to load it into my NTS-1 | If GitHub Releases contains `shimmer_revfx.ntkdigunit`, download that file. It is the ready-to-load user unit for NTS-1 Sound Librarian. |
| There is no `.ntkdigunit` in Releases | This repository is the source version. Download the whole repository with `Code` -> `Download ZIP`, or clone it with Git, then build it with the logue SDK. |
| I want to modify or study the DSP code | Clone the whole repository. `shimmer_revfx.cpp` alone is not enough to build the unit. |

The file you load into NTS-1 Sound Librarian is the `.ntkdigunit` file. `shimmer_revfx.cpp`, `manifest.json`, `Makefile`, `ld/`, and `tpl/` are source and build files.

## Compatibility

- KORG Nu:Tekt NTS-1 digital kit mkI
- logue SDK `nutekt-digital`
- API `1.1-0`
- This targets the original NTS-1 mkI, not the NTS-1 mkII

The KORG logue SDK README notes that user units built with SDK `1.1-0` require NTS-1 firmware `1.02` or later.

## Installation

If you have a ready-built `shimmer_revfx.ntkdigunit`:

1. Install KORG NTS-1 Sound Librarian.
2. Connect the NTS-1 to your computer over USB.
3. Import `shimmer_revfx.ntkdigunit` into the USER REVERB FX area.
4. Send the user data to the NTS-1.
5. Select `ShimmerVerb` from the NTS-1 reverb types.

KORG Sound Librarian manages custom oscillators and effects as user unit files. Drag the user unit into the matching list, then send it to the device.

## Controls

| NTS-1 control | Function |
| --- | --- |
| `TIME` | Reverb decay time |
| `DEPTH` | Dry/wet mix |
| `SHIFT + DEPTH` | Shimmer amount: how much octave-up signal is fed back into the tail |

Good starting points:

- `TIME`: 50-70%
- `DEPTH`: 30-45%
- `SHIFT + DEPTH`: 25-50%

Higher shimmer settings make the tail brighter and more pad-like. If the effect blooms too much or the high end becomes too strong, lower `SHIFT + DEPTH` first.

## Background

A shimmer reverb combines reverb with pitch shifting. The classic idea is simple: take part of the reverb tail, shift it up by one octave, and feed it back into the reverb. The result is a bright, rising ambience that feels less like a real room and more like a glowing synthetic space.

It works well on guitar, synth chords, arpeggios, drones, and external audio. Since the NTS-1 can process its audio input, it can act as a tiny desktop shimmer processor as well as a synth effect.

This implementation is not trying to be a huge studio reverb squeezed into a small box. The goal is a lightweight, playable, characterful shimmer that fits the NTS-1 mkI and stays useful while turning knobs in real time.

## How it works

Main DSP blocks:

- 4-line feedback delay network for the reverb body
- 2-stage allpass diffuser on the input
- dual-tap moving-delay octave-up pitch shifter
- high-passed shimmer feedback to keep the low end stable
- soft clipping in the feedback and output paths
- large delay buffers placed in `__sdram`

`TIME` controls both decay and damping. `DEPTH` controls the wet mix. `SHIFT + DEPTH` controls how much pitch-shifted signal returns to the feedback path.

The pitch shifter is intentionally lightweight. It is not a pristine standalone pitch-shifting algorithm; it is tuned for shimmer feedback. Depending on the settings, it can have a little motion and grain, which is part of the character of this unit.

## Build from source

You need KORG's logue SDK and the Arm toolchain.

Prepare the logue SDK:

```sh
git clone https://github.com/korginc/logue-sdk.git
cd logue-sdk
git submodule update --init
```

Clone this repository under `platform/nutekt-digital`:

```sh
cd platform/nutekt-digital
git clone https://github.com/ksd6700/shimmer_revfx_mki-NTS-1-digital-kit.git shimmer-revfx
cd shimmer-revfx
make install
```

If this repository lives somewhere else, pass `PLATFORMDIR`:

```sh
make PLATFORMDIR=/path/to/logue-sdk/platform/nutekt-digital install
```

The build output is:

```text
shimmer_revfx.ntkdigunit
```

Load that file with NTS-1 Sound Librarian.

## Project layout

| File / folder | Purpose |
| --- | --- |
| `shimmer_revfx.cpp` | Main DSP implementation |
| `manifest.json` | NTS-1 user unit metadata |
| `Makefile`, `project.mk` | logue SDK build setup |
| `ld/` | Linker files for a reverb effect user unit |
| `tpl/` | User unit entry template |
| `test-stubs/` | Minimal desktop syntax-check stubs. Not used for the real device build. |

## Development notes

Local syntax checks:

```sh
g++ -std=c++11 -Wall -Wextra -I test-stubs -fsyntax-only shimmer_revfx.cpp
python3 -m json.tool manifest.json
```

These checks do not create a device binary. Build the final `.ntkdigunit` with the KORG logue SDK toolchain.

## References

- KORG NTS-1 Sound Librarian: https://www.korg.com/us/products/dj/nts_1/librarian_contents.php
- KORG logue SDK: https://github.com/korginc/logue-sdk
- NTS-1 custom effects article: https://www.korg.com/us/products/dj/nts_1/custom_effects.php

## License

KORG template files are covered by `LICENSE-KORG-BSD-3-Clause.txt`.
