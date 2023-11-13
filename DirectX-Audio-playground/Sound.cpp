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

SoundList g_soundList; // サウンドリスト



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
	SoundList::iterator soundIt = g_soundList.begin(); // サウンドリストの先頭を取得

	// サウンドリストの末尾までループ
	while (soundIt != g_soundList.end())
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

/**
 * @brief サウンドデータの読み込み
 * @param[in] file 読み込むファイル名
 * @param[in] loop ループ再生するかどうか
 * @return サウンドバッファ
 */
XAUDIO2_BUFFER* Sound::LoadSound(const char* file, bool loop)
{
	SoundData data; // サウンドデータ

	SoundList::iterator it = g_soundList.find(file); // サウンドリストからファイル名を検索
	if (it != g_soundList.end()) // 検索結果が見つかった場合
	{
		// すでに読み込まれていたら、
		return &it->second.sound; // サウンドバッファを返す
	}

	// 拡張子ごとに読み込み処理
	HRESULT hr = E_FAIL;
	LPSTR ext = PathFindExtension(file); // ファイルパスから拡張子を取得
	if (ext != NULL)
	{
		if(memcmp(ext, ".wav", 4) == CMP_MATCH) // wavファイルの場合
		{
			hr = LoadWave(file, &data); // wavファイルの読み込み
		} 
		else if (memcmp(ext, "mp3", 4) == CMP_MATCH) // mp3ファイルの場合
		{
			hr = LoadMP3(file, &data); // mp3ファイルの読み込み
		}
	}
	if(FAILED(hr))
	{
		return NULL; // 読み込み失敗
	}

	// サウンドバッファの作成
	ZeroMemory(&data.sound, sizeof(data.sound)); // サウンドバッファの初期化
	// サウンドデータのバイト数
	data.sound.AudioBytes = data.bufSize;
	// サウンドデータの先頭アドレス
	data.sound.pAudioData = data.pBuffer;
	// ループ再生するかどうか
	if (loop == true)
	{
		data.sound.LoopCount = XAUDIO2_LOOP_INFINITE; // 無限ループ
	}
	data.sound.Flags = XAUDIO2_END_OF_STREAM; // 終端まで再生したら停止

	// サウンドリストに追加
	g_soundList.insert(SoundKey(file, data));
	it = g_soundList.find(file); // サウンドリストからファイル名を検索

	return &it->second.sound; // サウンドバッファを返す
}

/**
 * @brief サウンド再生
 * @param[in] pSound サウンドバッファ
 */
IXAudio2SourceVoice* Sound::PlaySound(XAUDIO2_BUFFER* pSound)
{
	HRESULT hr;	// 処理結果
	IXAudio2SourceVoice* pSource;	// ソース

	// 再生するデータを探索
	SoundList::iterator soundIt = g_soundList.begin(); // サウンドリストの先頭を取得
	while(soundIt != g_soundList.end()) // サウンドリストの末尾までループ
	{
		if (&soundIt->second.sound == pSound) // サウンドバッファが一致した場合
		{
			break; // ループを抜ける
		}
		++soundIt; // 次のサウンドデータへ
	}
	if (soundIt == g_soundList.end()) // サウンドバッファが見つからなかった場合
	{
		// 該当のデータなし
		return NULL;
	}

	// フォーマットを指定し、ソースを作成
	/*------------
	* WAVEFORMATEX 役割: WAVフォーマットを扱う
	* wFormatTag 役割: フォーマットタグ
	* nChannels 役割: チャンネル数
	  * 1: モノラル
	  *	2: ステレオ
	* wBitsPerSample 役割: 1 サンプルあたりのビット数
	  * 8: 8bit ... 128を無音とする 0~255
	  * 16: 16bit ... 0を無音とする -32768~32767
	* nSamplesPerSec 役割: サンプリングレート (1秒あたりのサンプリング数 : 単位: Hz)
	*-------------*/
	hr = m_pXAudio2->CreateSourceVoice(&pSource, &soundIt->second.format);
	if (FAILED(hr))
	{
		return NULL;
	}
	pSource->SubmitSourceBuffer(pSound); // ソースにバッファを設定

	// 再生
	pSource->Start();
	
	return pSource;
}

IXAudio2SourceVoice* Sound::StopSound(XAUDIO2_BUFFER* pSound)
{
	HRESULT hr;	// 処理結果
	IXAudio2SourceVoice* pSource;	// ソース

	// 停止させるデータを探索
	SoundList::iterator soundIt = g_soundList.begin(); // サウンドリストの先頭を取得
	while (soundIt != g_soundList.end()) // サウンドリストの末尾までループ
	{
		if (&soundIt->second.sound == pSound) // サウンドバッファが一致した場合
		{
			break; // ループを抜ける
		}
		++soundIt; // 次のサウンドデータへ
	}
	if (soundIt == g_soundList.end()) // サウンドバッファが見つからなかった場合
	{
		// 該当のデータなし
		return NULL;
	}

	// 停止
	pSource->Stop();

	return pSource;	
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

/**
 * @brief wavファイルの読み込み
 * @param[in] file 読み込むファイル
 * @param[out] pData サウンドデータ
 * @return 処理結果(success: S_OK, failed: E_FAIL) 
*/
HRESULT LoadWave(const char* file, Sound::SoundData* pData)
{
	HMMIO hMmio = NULL; // ファイルハンドル
	MMIOINFO mmioInfo; // ファイル情報
	MMRESULT mmRes; // ファイル読み込み結果

	// WAVEファイルオープン
	memset(&mmioInfo, 0, sizeof(MMIOINFO)); // ファイル情報の初期化
	hMmio = mmioOpen(const_cast<char*>(file), &mmioInfo, MMIO_READ); // ファイルを開く
	if (hMmio == NULL)
	{
		return E_FAIL;
	}

	// RIFFチャンク検索
	MMCKINFO riffChunk; // RIFFチャンク
	riffChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E'); // WAVEフォーマット
	mmRes = mmioDescend(hMmio, &riffChunk, NULL, MMIO_FINDRIFF); // RIFFチャンクを検索
	if (mmRes != MMSYSERR_NOERROR)
	{
		mmioClose(hMmio, 0); // ファイルを閉じる
		return E_FAIL;
	}

	// フォーマットチャンク検索
	MMCKINFO formatChunk; // フォーマットチャンク
	formatChunk.ckid = mmioFOURCC('f', 'm', 't', ' '); // フォーマットチャンク
	mmRes = mmioDescend(hMmio, &formatChunk, &riffChunk, MMIO_FINDCHUNK); // フォーマットチャンクを検索
	if (mmRes != MMSYSERR_NOERROR)
	{
		mmioClose(hMmio, 0); // ファイルを閉じる
		return E_FAIL;
	}

	// フォーマット取得
	DWORD formatSize = formatChunk.cksize; // フォーマットサイズ
	DWORD size = mmioRead(hMmio, reinterpret_cast<HPSTR>(&pData->format), formatSize); // フォーマットを読み込む
	if (size != formatSize)
	{
		mmioClose(hMmio, 0); // ファイルを閉じる
		return E_FAIL;
	}

	// RIFFチャンクに移動
	mmioAscend(hMmio, &formatChunk, 0); // フォーマットチャンクに移動

	// データチャンク検索
	MMCKINFO dataChunk; // データチャンク
	dataChunk.ckid = mmioFOURCC('d', 'a', 't', 'a'); // データチャンク
	mmRes = mmioDescend(hMmio, &dataChunk, &riffChunk, MMIO_FINDCHUNK); // データチャンクを検索
	if (mmRes != MMSYSERR_NOERROR)
	{
		mmioClose(hMmio, 0); // ファイルを閉じる
		return E_FAIL;
	}

	// データサイズ取得
	pData->bufSize = dataChunk.cksize; // データサイズ
	pData->pBuffer = new BYTE[pData->bufSize]; // データサイズ分のメモリを確保
	size = mmioRead(hMmio, reinterpret_cast<HPSTR>(pData->pBuffer), pData->bufSize); // データを読み込む
	if (size != dataChunk.cksize)
	{
		pData->bufSize = 0; // データサイズを0にする
		if (pData->pBuffer != NULL)
		{
			delete[] pData->pBuffer; // メモリの解放
			pData->pBuffer = NULL;
		}
		mmioClose(hMmio, 0); // ファイルを閉じる
		return E_FAIL;
	}

	// ファイルを閉じる
	mmioClose(hMmio, 0);
	return S_OK;
}

/**
 * @brief mp3ファイルの読み込み
 * @param[in] file 読み込むファイル
 * @param[out] pData サウンドデータ
 * @return 処理結果(success: S_OK, failed: E_FAIL)
 */
HRESULT LoadMP3(const char* file, Sound::SoundData* pData)
{
	HANDLE hFile; // ファイルポインタ
	DWORD readSize; // 読み込みサイズ

	// 読み込み
	hFile = CreateFile(
		file, // ファイル名	
		GENERIC_READ, // アクセスモード(読み込み)
		0, // 共有モード(なし)
		NULL, // セキュリティ記述子(なし)
		OPEN_EXISTING, // 作成方法(既存のファイルを開く)
		FILE_ATTRIBUTE_NORMAL, // ファイル属性(通常のファイル)
		NULL // テンプレートファイルのハンドル(なし)
	);
	if (hFile == INVALID_HANDLE_VALUE)	// ファイルが開けなかった場合
	{
		return E_FAIL; // 読み込み失敗
	}

	// ファイルフォーマットの読み込み
	Sound::MP3FormatInfo format; // フォーマット
	readSize = ReadMP3Format(hFile, &format); // フォーマットの読み込み
	if (readSize == 0) // 読み込み失敗
	{
		return E_FAIL; // 読み込み失敗
	}

	// サウンドフレームヘッダの読み込み
	Sound::MP3FrameInfo frame; // フレーム
	readSize = ReadMP3FrameHeader(hFile, format.offset, &frame); // フレームヘッダの読み込み
	if (readSize == 0) // 読み込み失敗
	{
		return E_FAIL; // 読み込み失敗
	}

	// サウンドデータの読み込み
	readSize = ReadMP3Data(hFile, format.offset, format.dataSize, &frame, pData); // サウンドデータの読み込み
	if (readSize == 0) // 読み込み失敗
	{
		return E_FAIL; // 読み込み失敗
	}

	return S_OK; // 読み込み成功
}

/**
 * @brief MP3フォーマットチェック
 * @param[in] hFile ファイルハンドル
 * @param[out] pFormat フォーマット情報
 * @return データサイズ
 */
DWORD ReadMP3Format(HANDLE hFile, Sound::MP3FormatInfo* pFormat)
{
	return 0;
}

DWORD ReadMP3FrameHeader(HANDLE hFile, DWORD seek, Sound::MP3FrameInfo* pFrame)
{
	return 0;
}

DWORD ReadMP3Data(HANDLE hFile, DWORD seek, DWORD size, Sound::MP3FrameInfo* pFrame, Sound::SoundData* pData)
{
	return 0;
}
