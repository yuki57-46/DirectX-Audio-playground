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
	hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
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
		if(memcmp(ext, ".wav", 4) == 0) // wavファイルの場合
		{
			hr = LoadWave(file, &data); // wavファイルの読み込み
		} 
		else if (memcmp(ext, ".mp3", 4) == 0) // mp3ファイルの場合
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
	//IXAudio2SourceVoice* pSource = NULL;	// ソース

	// 停止させるデータを探索
	SoundList::iterator soundIt = g_soundList.begin(); // サウンドリストの先頭を取得
	while (soundIt != g_soundList.end()) // サウンドリストの末尾までループ
	{
		if (&soundIt->second.sound == pSound) // サウンドバッファが一致した場合
		{
			//pSource = soundIt->second.pSource; // ソースを取得
			
			// 停止
			pSound->pAudioData = NULL;
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
	//pSource->Stop();

	return NULL;
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
	DWORD readSize; // 読み込みサイズ

	// MP3ファイルサイズの取得
	DWORD fileSize = GetFileSize(hFile, NULL); // ファイルサイズの取得

	// ヘッダー/フッターの有無を確認
	/*-------
	* ID3v1
	* -末尾にタグがある or タグなし
	* [MP3データ]
	* 	or
	* 	[MP3データ]["TAG"(3byte)][タグ情報(125byte)]
	* ID3v2
	* -先頭にタグがある
	*  ["ID3"(3byte)][バージョン(2byte)][フラグ(1byte)][タグサイズ(4byte)][拡張ヘッダ][MP3データ]
	*--------*/
	const BYTE ID3V2_HEADER_SIZE = 10; // ID3v2ヘッダサイズ
	BYTE header[ID3V2_HEADER_SIZE]; // ヘッダー情報
	ReadFile(hFile, header, sizeof(header), &readSize, NULL); // ヘッダー情報の読み込み

	// タグをチェックし、MP3データの位置、サイズを計算
	const char *ID3V1_TAG = "TAG"; // ID3v1タグ
	const char *ID3V2_TAG = "ID3"; // ID3v2タグ
	const BYTE MP3_TAG_SIZE = 3; // MP3タグサイズ
	if (memcmp(header, ID3V2_TAG, MP3_TAG_SIZE) == CMP_MATCH)
	{
		// ID3v2ヘッダー情報解析
		/*-----------------
		* [拡張ヘッダ]のデータサイズについて
		*  [タグサイズ]のデータ構造
		*    [0AAAAAAA][0BBBBBBB][0CCCCCCD][0DDDDDDD]
		*  実際のデータサイズ
		*	 0x0000AAAAAAABBBBBBBCCCCCCCDDDDDDD
		*
		* - データの最上位ビットは必ず0
		*	実際のサイズは、0を省いて詰めたもの
		*-----------------*/
		DWORD exHeaderSize = 
			(header[6] << 21) |
			(header[7] << 14) |
			(header[8] << 7) |
			(header[9] << 0);
		pFormat->offset = exHeaderSize + ID3V2_HEADER_SIZE; // MP3データのオフセット
		pFormat->dataSize = fileSize - pFormat->offset; // MP3データのサイズ
	}
	else
	{
		// ID3v1フッター情報解析
		const BYTE ID3V1_FOOTER_SIZE = 128; // ID3v1フッターサイズ
		BYTE tag[MP3_TAG_SIZE]; // タグ情報
		SetFilePointer(hFile, fileSize - ID3V1_FOOTER_SIZE, NULL, FILE_BEGIN); // ファイル末尾へ移動
		ReadFile(hFile, tag, MP3_TAG_SIZE, &readSize, NULL); // タグ情報の読み込み
		pFormat->offset = 0; // MP3データのオフセット
		if (memcmp(tag, ID3V1_TAG, MP3_TAG_SIZE) == CMP_MATCH)
		{
			pFormat->dataSize = fileSize - 128; // MP3データのサイズ
		}
		else
		{
			pFormat->dataSize = fileSize; // MP3データのサイズ
		}
	}

	return pFormat->dataSize; // データサイズを返す
}

/**
 * @brief MP3フレームヘッダの読み込み
 * @param[in] hFile ファイルハンドル
 * @param[in] seek フレーム読み込み位置
 * @param[out] pFrame フレーム情報
 * @return 読み込みサイズ
 */
DWORD ReadMP3FrameHeader(HANDLE hFile, DWORD seek, Sound::MP3FrameInfo* pFrame)
{
	DWORD readSize; // 読み込みサイズ

	// フレームヘッダから情報を取得
	/*-----------------
	* フレームヘッダの構造
	* [フレームヘッダ(4byte)][データ]
	* [フレームヘッダ(4byte)][データ]	
	* [フレームヘッダ(4byte)][データ]
	* ...(繰り返し)
	*-----------------*/
	// MP3データ位置へ移動
	SetFilePointer(hFile, seek, NULL, FILE_BEGIN);

	/*-----------------
	* フレームヘッダ情報
	* https://web.archive.org/web/20200208193543/cactussoft.co.jp/Sarbo/divMPeg3UnmanageHeader.html
	* [AAAAAAAA][AAABBCCD][EEEEFFGH][IIJJKLMM]
	* A: 同期ビット(必ず1)
	* B: MP3のバージョン
	*   00: MPEG Version 2.5 (無効)
	*   01: 予約
	*   10: MPEG Version 2 (無効)
	*   11: MPEG Version 1
	* C: レイヤー
	*   00: 予約
	*   01: Layer III
	*   10: Layer II
	*   11: Layer I
	* D: CRCフラグ
	*   0: 16bit CRC誤り検出の支援
	*   1: CRCなし
	* E: ビットレート
	* F: サンプリングレート
	* G: パディング(フレームの最後のデータが0xFFだった場合、
				次のフレームの先頭とつながるため、フレームの最後にNULLを挿入させる
				実際にデータが挿入されたかどうかの情報を持つ)
	* H: 拡張(未使用)
	* I: チャンネルモード
	*   00: ステレオ
	*   01: ジョイントステレオ(ステレオ)
	*   10: デュアルチャンネル(ステレオ)
	*   11: モノラル
	* J: 拡張
	* K: 著作権の有無
	*   0: 著作権なし
	*   1: 著作権あり
	* L: オリジナル/コピー
	*   0: オリジナルからコピーされたメディア
	*   1: オリジナルのメディア
	* M: 強調
	*   00: 強調なし
	*   01: 50/15ms 強調
	*   10: 未使用
	*   11: CCIT J.17 強調
	*
	* - 最低限必要な情報は B, C, E, F, G, I
	*/
	const BYTE FRAME_HEADER_SIZE = 4; // フレームヘッダサイズ
	BYTE frameHeader[FRAME_HEADER_SIZE]; // フレームヘッダ
	ReadFile(hFile, frameHeader, FRAME_HEADER_SIZE, &readSize, NULL); // フレームヘッダの読み込み

	// 同期ビットのチェック
	if(!(frameHeader[0] == 0xFF && (frameHeader[1] & 0xE0) == 0xE0))
	{
		return 0; // 読み込み失敗
	}

	// バージョン
	BYTE version = (frameHeader[1] >> 3) & 0b11;

	// レイヤー
	BYTE layer = (frameHeader[1] >> 1) & 0b11;

	// ビットレート(単位: kbps (kbit/s))
	const int bitRateTable[][16] = {
		// MPEG 1, Layer 1
		{ 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, -1 },
		// MPEG 1, Layer 2
		{ 0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384, -1 },
		// MPEG 1, Layer 3
		{ 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, -1 },
		// MPEG 2, Layer 1
		{ 0, 32, 48, 56, 64, 80, 96, 112, 128, 144, 160, 176, 192, 224, 256, -1 },
		// MPEG 2/2.5, Layer 2,3
		{ 0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, -1 }
	};
	BYTE bitRateTableIndex;	// ビットレートテーブルのインデックス
	if (version == 0b11)
	{
		bitRateTableIndex = 0b11 - layer; // MPEG 1
	}
	else if (version == 0b10 && layer == 0b11)
	{
		bitRateTableIndex = 3; // MPEG 2, Layer 1
	}
	else
	{
		bitRateTableIndex = 4; // MPEG 2/2.5
	}
	WORD bitRate = bitRateTable[bitRateTableIndex][frameHeader[2] >> 4]; // ビットレート

	// サンプリングレート(単位: Hz)
	const int samplingRateTable[][4] = {
		// MPEG 1
		{ 44100, 48000, 32000, -1 },
		// MPEG 2
		{ 22050, 24000, 16000, -1 },
		// MPEG 2.5
		{ 11025, 12000, 8000, -1 }
	};
	BYTE samplingRateTableIndex; // サンプリングレートテーブルのインデックス
	switch (version)
	{
	case 0b00:
		samplingRateTableIndex = 2; // MPEG 2.5
		break;
	case 0b10:
		samplingRateTableIndex = 1; // MPEG 2
		break;
	case 0b11:
		samplingRateTableIndex = 0; // MPEG 1
		break;
	default:
		return E_FAIL; // 読み込み失敗
	}
	WORD samplingRate = samplingRateTable[samplingRateTableIndex][(frameHeader[2] >> 2) & 0b11]; // サンプリングレート

	// パディング
	BYTE padding = (frameHeader[2] >> 1) & 0b01;

	// チャンネル
	BYTE channel = frameHeader[3] >> 6;

	// フレームサイズ
	// ビットレートはkbps(kbit/sec)なので、bps(bit/sec)に変換するために1000倍する
	const int samplePerFrame[][4] =
	{
		// layer 1
		{ 384, 384, 384, -1 },
		// layer 2
		{ 1152, 1152, 1152, -1 },
		// layer 3
		{ 1152, 576, 576, -1 }
	};
	WORD frameBlockSize = ((samplePerFrame[0b11 -layer][samplingRateTableIndex] * bitRate * 1000 / 8) / samplingRate) + padding; // フレームサイズ

	// 取得した情報をもとにMPEGLAYER3WAVEFORMATにデータ設定
	/*-----------------
	 * MPEGLAYER3WAVEFORMAT 役割: MP3フォーマットを扱う
	 * https://web.archive.org/web/20200220093522/cactussoft.co.jp/Sarbo/divMPeg3UnmanageStruct.html
	 * - mpegオーディオレイヤー3(mp3)を扱うために拡張されたWAVEFORMAT構造体
	 * .wfx.cbsize 			役割: 構造体の拡張サイズ, MPEGLAYER3_WFX_EXTRA_BYTESを指定
	 * .wfx.nChannels 		役割: チャンネル数 (1: モノラル, 2: ステレオ)
	 * .wfx.wFormatTag 		役割: mp3を表す, WAVE_FORMAT_MPEGLAYER3を指定
	 * .wfx.nBlockAlign 	役割: 1を指定
	 * .wfx.wBitsPerSample 	役割: 0を指定
	 * .wfx.nSamplesPerSec 	役割: サンプリングレート
	 * .wfx.nAvgBytesPerSec 役割: 1秒間に消費するデータサイズを指定。ビットレートを8で割った値になる
	 * wID 					--: MPEGLAYER3_ID_MPEGを指定
	 * fdwFlags				役割: パディングを挿入があれば指定 (MPEGLAYER3_FLAG_PADDING_ISO : 必要に応じてパディングを挿入 , MPEGLAYER3_FLAG_PADDING_ON : 常にパディングを挿入 , MPEGLAYER3_FLAG_PADDING_OFF : パディングを挿入しない)
	 * nFramesPerBlock		役割: 1つのブロックに配置するフレーム数を指定
	 * nBlockSize			役割: ブロックサイズの指定 (フレームサイズ * フレーム数)
	 * nCodecDelay			--: 1393(0x571)を指定
	 * -----------------*/
	
	// channel
	// samplerate
	// bitrate
	// padding
	// frameBlockSize
	pFrame->channel = channel == 0x11 ? 1 : 2; // チャンネル数 (1: モノラル, 2: ステレオ)
	pFrame->sampleRate = samplingRate; // サンプリングレート
	pFrame->bitRate = bitRate; // ビットレート
	pFrame->padding = padding; // パディング
	pFrame->frameSize = frameBlockSize; // フレームサイズ

	return pFrame->frameSize; // 読み込みサイズを返す
}

/**
 * @brief MP3サウンドデータの読み込み
 * @param[in] hFile ファイルポインタ
 * @param[in] seek フレーム読み込み位置
 * @param[in] size 読み込みサイズ
 * @param[in] pFrame フレーム情報
 * @param[out] pData サウンドデータ
 * @return 読み込みサイズ
 */
DWORD ReadMP3Data(HANDLE hFile, DWORD seek, DWORD size, Sound::MP3FrameInfo* pFrame, Sound::SoundData* pData)
{
	// 変換フォーマット作成
	MPEGLAYER3WAVEFORMAT mp3WaveFormat; // MP3フォーマット
	mp3WaveFormat.wfx.cbSize = MPEGLAYER3_WFX_EXTRA_BYTES; // 拡張サイズ
	mp3WaveFormat.wfx.nChannels = pFrame->channel; // チャンネル数
	mp3WaveFormat.wfx.wFormatTag = WAVE_FORMAT_MPEGLAYER3; // mp3を表す
	mp3WaveFormat.wfx.nBlockAlign = 1; // 1を指定
	mp3WaveFormat.wfx.wBitsPerSample = 0; // 0を指定
	mp3WaveFormat.wfx.nSamplesPerSec = pFrame->sampleRate; // サンプリングレート
	mp3WaveFormat.wfx.nAvgBytesPerSec = (pFrame->bitRate * 1000) / 8; // 1秒間に消費するデータサイズを指定。ビットレートを8で割った値になる

	// フォーマットを指定し、ソースを作成
	mp3WaveFormat.wID = MPEGLAYER3_ID_MPEG; // MPEGLAYER3_ID_MPEGを指定
	mp3WaveFormat.fdwFlags = pFrame->padding ? MPEGLAYER3_FLAG_PADDING_ON : MPEGLAYER3_FLAG_PADDING_OFF; // パディングを挿入があれば指定 (MPEGLAYER3_FLAG_PADDING_ISO : 必要に応じてパディングを挿入 , MPEGLAYER3_FLAG_PADDING_ON : 常にパディングを挿入 , MPEGLAYER3_FLAG_PADDING_OFF : パディングを挿入しない)
	mp3WaveFormat.nFramesPerBlock = 1; // 1つのブロックに配置するフレーム数を指定
	mp3WaveFormat.nBlockSize = static_cast<WORD>(pFrame->frameSize * mp3WaveFormat.nFramesPerBlock); // ブロックサイズの指定 (フレームサイズ * フレーム数)
	mp3WaveFormat.nCodecDelay = 0x571; // 1393(0x571)を指定

	// mp3をwavに変換可能かチェック
	// 可能であれば、wavFormatへデータを設定
	WAVEFORMATEX wavFormat; // wavフォーマット
	wavFormat.wFormatTag = WAVE_FORMAT_PCM; // PCMを指定
	MMRESULT mmr; // 処理結果
	mmr = acmFormatSuggest(NULL, &mp3WaveFormat.wfx, &wavFormat, sizeof(WAVEFORMATEX), ACM_FORMATSUGGESTF_WFORMATTAG); // 変換可能かチェック
	if (mmr != MMSYSERR_NOERROR) // 変換不可
	{
		return 0; // 読み込み失敗
	}

	// ACMストリームを開く
	// mp3をwavへフォーマット変換
	HACMSTREAM has; // ACMストリーム
	mmr = acmStreamOpen(&has, NULL, &mp3WaveFormat.wfx, &wavFormat, NULL, NULL, NULL, 0); // ACMストリームを開く

	// MP3のブロックサイズからWAVE形式へデコード後のサイズを取得
	DWORD waveBlockSize; // WAVE形式のブロックサイズ
	acmStreamSize(has, size, &waveBlockSize, ACM_STREAMSIZEF_SOURCE); // WAVE形式のブロックサイズを取得

	// 変換データセット
	ACMSTREAMHEADER ash = { 0 }; // ACMストリームヘッダ
	ash.cbStruct = sizeof(ACMSTREAMHEADER); // 構造体サイズ
	ash.cbSrcLength = size; // 変換元データサイズ
	ash.pbSrc = new BYTE[ash.cbSrcLength]; // 変換元データ
	ash.cbDstLength = waveBlockSize; // 変換後データサイズ
	ash.pbDst = new BYTE[ash.cbDstLength]; // 変換後データ

	// デコード
	acmStreamPrepareHeader(has, &ash, 0); // ヘッダーの準備
	DWORD readSize; // 読み込みサイズ
	SetFilePointer(hFile, seek, NULL, FILE_BEGIN); // ファイルポインタを移動
	ReadFile(hFile, ash.pbSrc, ash.cbSrcLength, &readSize, NULL); // ファイルからデータを読み込む
	mmr = acmStreamConvert(has, &ash, 0); // デコード
	acmStreamUnprepareHeader(has, &ash, 0); // ヘッダーの解放
	acmStreamClose(has, 0); // ACMストリームを閉じる

	// wavデータコピー
	if (ash.cbDstLengthUsed > 0)
	{
		pData->bufSize = ash.cbDstLengthUsed; // データサイズ
		pData->pBuffer = new BYTE[pData->bufSize]; // データサイズ分のメモリを確保
		pData->format = wavFormat; // フォーマット
		memcpy_s(pData->pBuffer, pData->bufSize, ash.pbDst, ash.cbDstLengthUsed); // データをコピー
	}

	// メモリ解放
	delete[] ash.pbSrc; // 変換元データ
	delete[] ash.pbDst; // 変換後データ

	CloseHandle(hFile); // ファイルを閉じる

	return ash.cbDstLengthUsed; // 読み込みサイズを返す
}
