//
// File: FileLoader.cpp
// ファイルを読み込むクラス
//
#include "FileLoader.hpp"

Sound g_Sound; // サウンドクラスのインスタンス

// 定数宣言
LPCSTR OpenFileWindowTitle = "FilePath";

// プロトタイプ宣言
LPCSTR ConvertPWSTRtoLPCSTR(PWSTR pwsz);

FileLoader::FileLoader()
    : m_pFileOpenDialog(NULL)
    , m_filePath()
{
    g_Sound.Init(); // サウンドクラスの初期化
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

}

FileLoader::~FileLoader()
{
    g_Sound.Uninit(); // サウンドクラスの終了
    CoUninitialize();
}

void FileLoader::OpenFile(HWND hWnd)
{
    IFileOpenDialog* pFileOpen; // ファイル選択ダイアログのインスタンス
    pFileOpen = NULL;
    // ファイル選択ダイアログのインスタンスを作成
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, (void**)&pFileOpen);

    // ファイル選択ダイアログの設定
    if(SUCCEEDED(hr))
    {
        hr = pFileOpen->Show(hWnd); // ファイル選択ダイアログを開く

        // ファイル選択ダイアログでOKボタンが押されたら 
        if(SUCCEEDED(hr))
        {
            IShellItem* pItem; // 選択されたファイルのインスタンス
            hr = pFileOpen->GetResult(&pItem); // 選択されたファイルのインスタンスを取得

            
            if(SUCCEEDED(hr))
            {
                PWSTR filePath; // ファイルパス
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath); // ファイルパスを取得


                if(SUCCEEDED(hr))
                {
                    // 選択されたファイルのパスはfilePathに格納されている
                    // ここでファイルの処理を行う
                    // ファイルパスを表示
                    LPCSTR lpFilePath = ConvertPWSTRtoLPCSTR(filePath);  // ファイルパスをLPCSTRに変換
                    MessageBoxA(hWnd, lpFilePath, OpenFileWindowTitle, MB_OK);
                    m_pSound = g_Sound.LoadSound(lpFilePath, false); // サウンドファイルの読み込み
                    m_pSourceVoice = g_Sound.PlaySound(m_pSound); // サウンドの再生



                    delete[] lpFilePath; // メモリ解放
                    CoTaskMemFree(filePath); // メモリ解放
                }

                pItem->Release(); // メモリ解放
            }
        }

        m_pSourceVoice = g_Sound.StopSound(m_pSound); // サウンドの停止

        pFileOpen->Release(); // メモリ解放
    }
}

const wchar_t* FileLoader::GetFilePath() const
{
    return m_filePath;
}

void FileLoader::LoadDropFile(HWND hWnd, HDROP hDrop)
{
    // ドロップされたファイルの数を取得
    UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

    // ドロップされたファイルの数だけループ
    for(UINT i = 0; i < fileCount; i++)
    {
        // ファイルパスの長さを取得
        UINT filePathLength = DragQueryFile(hDrop, i, NULL, 0); 
        // ファイルパスの長さ分のメモリを確保
        WCHAR* filePath = new WCHAR[filePathLength + 1];
        // ファイルパスを取得
        DragQueryFileW(hDrop, i, filePath, filePathLength + 1);

        LPCSTR lpFilePath = ConvertPWSTRtoLPCSTR(filePath);
        // filePathにはドロップされたファイルのパスが格納されている
        // ここでファイルの処理を行う
        // ファイルパスを表示
        MessageBoxA(hWnd, lpFilePath, OpenFileWindowTitle, MB_OK);
        m_pSound = g_Sound.LoadSound(lpFilePath, false); // サウンドファイルの読み込み
        g_Sound.PlaySound(m_pSound);

        delete[] filePath; // メモリ解放
    }

    g_Sound.StopSound(m_pSound); // サウンドの停止

    DragFinish(hDrop); // メモリ解放
}


LPCSTR ConvertPWSTRtoLPCSTR(PWSTR pwsz)
{
    // 必要なバッファサイズを計算
    int bufferSize = WideCharToMultiByte( 932, 0, pwsz, -1, NULL, 0, NULL, NULL);

    // バッファを確保
    char* buffer = new char[bufferSize];

    // 変換
    WideCharToMultiByte( 932, 0, pwsz, -1, buffer, bufferSize, NULL, NULL);

    // LPCSTRに変換
    LPCSTR lpcstr = (LPCSTR)buffer;


    return lpcstr;
}