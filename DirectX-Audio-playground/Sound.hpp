#ifndef SOUND_HPP
#define SOUND_HPP

#include <xaudio2.h>

#pragma comment(lib, "xaudio2.lib")

// サウンドクラスの定義
class Sound
{
public:
    HRESULT Init();
    void Uninit();

    // サウンドファイルの読み込み
    XAUDIO2_BUFFER* LoadSound(const char* file, bool loop = false);
    // サウンドの再生
    #undef PlaySound
    IXAudio2SourceVoice* PlaySound(XAUDIO2_BUFFER* pSound);
    // サウンドの停止
    IXAudio2SourceVoice* StopSound(XAUDIO2_BUFFER* pSound);
    // サウンドの一時停止
    void PauseSound(IXAudio2SourceVoice* pSourceVoice);
    // サウンドの再開
    void ResumeSound(IXAudio2SourceVoice* pSourceVoice);
    // サウンドのボリュームの設定
    void SetVolume(IXAudio2SourceVoice* pSourceVoice, float volume);
    // サウンドのピッチの設定
    void SetPitch(IXAudio2SourceVoice* pSourceVoice, float pitch);
    // サウンドの
    
private:

    IXAudio2* m_pXAudio2;
    IXAudio2MasteringVoice* m_pMasteringVoice;


public:
    struct SoundData    
    {
        WAVEFORMATEX    format; // WAVフォーマット
        BYTE            *pBuffer; // サウンドデータ
        DWORD           bufSize; // データサイズ
        XAUDIO2_BUFFER  sound;  // サウンドバッファ
        IXAudio2SourceVoice* pSource; // サウンドソース
    };
    
    struct MP3FormatInfo
    {
        DWORD offset;   // オフセット
        DWORD dataSize; // データサイズ
    };

    struct MP3FrameInfo
    {
        BYTE channel;  // チャンネル数
        BYTE padding; // パディング
        DWORD sampleRate; // サンプルレート
        DWORD bitRate; // ビットレート
        DWORD frameSize; // フレームサイズ
    };
    

    
    
};


#endif //! SOUND_HPP
