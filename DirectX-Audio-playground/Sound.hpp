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
    void StopSound(IXAudio2SourceVoice* pSourceVoice);
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
};


#endif //! SOUND_HPP
