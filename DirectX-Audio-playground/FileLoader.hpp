// 
// File: FileLoader.h
// ファイルを読み込むクラス
//
#pragma once

#include <Windows.h>
#include <ShObjIdl.h> // IFileOpenDialog(ファイル選択ダイアログ)を使うために必要
#include "Sound.hpp"

class FileLoader
{
public:
    FileLoader();
    ~FileLoader();

    // ファイル選択ダイアログを開く
    void OpenFile(HWND hWnd);

    // ファイルパスを取得する
    const wchar_t* GetFilePath() const;

    void LoadDropFile(HWND hWnd, HDROP hDrop);

private:
    // ファイル選択ダイアログのインスタンス
    IFileOpenDialog* m_pFileOpenDialog;

    // ファイルパス
    wchar_t m_filePath[MAX_PATH];
    
    IXAudio2SourceVoice* m_pSourceVoice; // サウンドソース
    XAUDIO2_BUFFER* m_pSound; // サウンドバッファ

};
