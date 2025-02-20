# Sound.hppの説明

## Soundクラス

### Soundクラスの概要

Soundクラスは、音声を再生/停止するためのクラスです。

### Soundクラスの詳細

#### Soundの初期化

```cpp
Sound::Init();
```

#### Soundの終了処理

```cpp
Sound::Uninit();
```

#### Soundの読み込み

```cpp
Sound::LoadSound(const char* file, bool loop = false);
```

fileには、読み込む音声ファイルのパスを指定します。
loopは省略可能です。
loopにtrueを指定すると、ループ再生されます。

#### Soundの再生

```cpp
Sound::PlaySound(XAUDIO2_BUFFER* pSound);
```

pSoundには、LoadSoundで読み込んだ音声ファイルを指定します。

#### Soundの停止

```cpp
Sound::StopSound(XAUDIO2_BUFFER* pSound);
```

PlaySoundで再生した音声を停止します。
pSoundには、LoadSoundで読み込んだ音声ファイルを指定します。

