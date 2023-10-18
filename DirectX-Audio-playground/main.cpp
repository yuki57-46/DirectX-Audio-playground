// main.cpp 
//
// DirectX 11/12 Audio Playground
// DirectX 11/12 のサウンドシステムを実験する
//

#include <Windows.h>
#include <iostream>
#include <string>

#pragma comment(lib, "winmm.lib")

const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;

// プロトタイプ宣言
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// メイン関数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wcex;
    ZeroMemory(&wcex, sizeof(wcex)); // 構造体の初期化
    wcex.hInstance = hInstance; // インスタンスハンドル
    wcex.lpszClassName = "DirectX-Audio-playground";    // ウィンドウクラス名
    wcex.lpfnWndProc = WndProc; // ウィンドウプロシージャ
    wcex.style = CS_HREDRAW | CS_VREDRAW; // ウィンドウのサイズが変更されたときに再描画する
    wcex.cbSize = sizeof(WNDCLASSEX); // 構造体のサイズ
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION); // アイコン
    wcex.hIconSm = wcex.hIcon; // タスクバーのアイコン
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW); // マウスカーソル
    wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); // ウィンドウの背景色
    wcex.lpszMenuName = NULL; // メニュー

    // ウィンドウクラスの登録
    if(!RegisterClassEx(&wcex))
    {
        MessageBoxEx(NULL, "(ウィンドウクラスの登録に失敗しました。)", "(エラー)", MB_OK, NULL);
        
        return 0;
    }

    // ウィンドウの作成
    HWND hWnd = CreateWindowEx(
        WS_EX_OVERLAPPEDWINDOW, // 拡張スタイル
        wcex.lpszClassName, "DirectX-Audio-playground", // タイトルバーのテキスト(ウィンドウクラス名, ウィンドウ名)
        WS_CAPTION | WS_SYSMENU , // ウィンドウスタイル
        CW_USEDEFAULT, CW_USEDEFAULT, // ウィンドウの表示位置(x, y)
        WINDOW_WIDTH, WINDOW_HEIGHT, // ウィンドウの幅と高さ
        HWND_DESKTOP, NULL, hInstance, NULL // 親ウィンドウのハンドル, メニューハンドル, インスタンスハンドル, 作成したウィンドウのハンドル
    );

    if(hWnd == NULL)
    {
        MessageBoxEx(NULL, "(ウィンドウの作成に失敗しました。)", "(エラー)", MB_OK, NULL);
        return 0;
    }

    // ウィンドウの表示
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    
    //--- 各種初期化処理 ---//
    timeBeginPeriod(1); // 分解能を1msに設定
    //HRESULT hr;


    //--- メッセージループ ---//
    MSG message;
    DWORD oldTime = timeGetTime(); // 前回の時間
    DWORD nowTime = oldTime; // 現在の時間
    while(1)
    {
        //--- 時間の更新 ---//
        if(PeekMessage(&message, NULL, 0, 0, PM_NOREMOVE)) // メッセージがある場合
        {
            //--- メッセージの取得 ---//
            if(!GetMessage(&message, NULL, 0, 0)) 
            {
                break; // メッセージを取得
            } 
            else
            {
                TranslateMessage(&message); // キー入力メッセージを処理
                DispatchMessage(&message); // メッセージのディスパッチ
            }
        }
        else // メッセージがない場合
        {
            // 経過時間の判定
            nowTime = timeGetTime();
            if(nowTime - oldTime >= 1000 / 60) // 1/60秒経過していたら
            {
                //--- 更新処理 ---//

                //--- 描画処理 ---//


                //--- 時間の更新 ---//
                oldTime = nowTime;
            }
        }
    }

    //--- 終了処理 ---//

    timeEndPeriod(1); // 分解能を戻す
    
    UnregisterClass(wcex.lpszClassName, hInstance); // ウィンドウクラスの登録解除

    return 0;
}

// ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
    case WM_DESTROY: // ウィンドウが破棄されたとき
		PostQuitMessage(0); // 終了メッセージを送る
		break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam); // デフォルトの処理
}