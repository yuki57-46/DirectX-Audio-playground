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

//-----------------
// プロトタイプ宣言
//-----------------
HRESULT LoadWave(const char* file, Sound::SoundData *pData);
HRESULT LoadMP3(const char* file, Sound::SoundData *pData);
DWORD ReadMP3Format(HANDLE hFile, Sound::MP3FormatInfo *pFormat);
DWORD ReadMP3FrameHeader(HANDLE hFile, DWORD seek, Sound::MP3FrameInfo *pFrame);
DWORD ReadMP3Data(HANDLE hFile, DWORD seek, DWORD size, Sound::MP3FrameInfo *pFrame, Sound::SoundData *pData);

//--------------
// 定数定義
//--------------
const BYTE CMP_MATCH = 0; // memcmpの戻り値が一致した場合

//-----------------
// グローバル変数
//-----------------

// std::pair 役割: 2つの値をまとめて扱う
// std::string 役割: 文字列を扱う
// Sound::SoundData 役割: サウンドデータを扱う
// SoundKey 役割: サウンドデータを扱う
typedef std::pair<std::string, Sound::SoundData> SoundKey;
// std::map 役割: 連想配列を扱う
// SoundKey 役割: サウンドデータを扱う
// Sound::SoundData 役割: サウンドデータを扱う
// SoundList 役割: サウンドデータを扱う
typedef std::map<std::string, Sound::SoundData> SoundList;





/**
 * @brief 初期化処理
 * @return 処理結果(sccess: S_OK, failed: E_FAIL)
 */
HRESULT Sound::Init()
{	
	HRESULT hr = E_FAIL;

	// COMオブジェクト初期化
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		return hr;
	}

	// XAUDIO2初期化
	hr = XAudio2Create(&m_pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR); // XAudio2オブジェクトの作成
	// ↑ 引数(IXAudio2** ppXAudio2, UINT32 Flags, XAUDIO2_PROCESSOR XAudio2ProcessorId)
	// IXAudio2** ppXAudio2: XAudio2オブジェクトのアドレス: XAudio2オブジェクトを格納する変数のアドレス
	// UINT32 Flags: フラグ: 0(なし) or XAUDIO2_DEBUG_ENGINE(デバッグモード) and more...
	// XAUDIO2_PROCESSOR XAudio2ProcessorId: プロセッサーID: XAUDIO2_DEFAULT_PROCESSOR(デフォルト) or XAUDIO2_ANY_PROCESSOR(任意) and more...
	if (FAILED(hr))
	{
		return hr;
	}

	// マスターボイス(出力先)の作成
	hr = m_pXAudio2->CreateMasteringVoice(&m_pMasteringVoice);

	return hr; // 処理結果を返す
}

/**
 * @brief 終了処理
 */
void Sound::Uninit()
{
	SoundList::iterator soundIt = m_SoundList.begin(); // サウンドリストの先頭を取得

	// サウンドリストの末尾までループ
	while (soundIt != m_SoundList.end())
	{
		delete[] soundIt->second.pBuffer; // サウンドデータの解放
		soundIt++; // 次のサウンドデータへ
	}

	if (m_pMasteringVoice != NULL)
	{
		m_pMasteringVoice->DestroyVoice(); // マスターボイスの破棄
		m_pMasteringVoice = NULL;
	}

	if (m_pXAudio2 != NULL)
	{
		m_pXAudio2->Release(); // XAudio2オブジェクトの解放
		m_pXAudio2 = NULL;
	}
	
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
