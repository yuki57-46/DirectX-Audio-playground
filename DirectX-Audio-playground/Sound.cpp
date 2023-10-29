#include "Sound.hpp"

#include <mmsystem.h> // wavファイルの読み込みに使用
#include <mmreg.h> // wavファイルの読み込みに使用
#include <MSAcm.h>  // ファイルの読み込みに使用
#include <Shlwapi.h> // ファイルパスの取得に使用
#include <vector> // std::vector 役割: 配列のように扱えるコンテナ
#include <map> // std::map 役割: 配列のように扱える連想配列
#include <string> // std::string 役割: 文字列を扱う

#pragma comment(lib, "winmm.lib") // wavファイルの読み込みに使用
#pragma comment(lib, "msacm32.lib") // ファイルの読み込みに使用
#pragma comment(lib, "shlwapi.lib") // ファイルパスの取得に使用

HRESULT Sound::Init()
{
	return E_NOTIMPL;
}

void Sound::Uninit()
{
}

XAUDIO2_BUFFER* Sound::LoadSound(const char* file, bool loop)
{
	return nullptr;
}

IXAudio2SourceVoice* Sound::PlaySound(XAUDIO2_BUFFER* pSound)
{
	return nullptr;
}

void Sound::StopSound(IXAudio2SourceVoice* pSourceVoice)
{
}

void Sound::PauseSound(IXAudio2SourceVoice* pSourceVoice)
{
}

void Sound::ResumeSound(IXAudio2SourceVoice* pSourceVoice)
{
}

void Sound::SetVolume(IXAudio2SourceVoice* pSourceVoice, float volume)
{
}

void Sound::SetPitch(IXAudio2SourceVoice* pSourceVoice, float pitch)
{
}
