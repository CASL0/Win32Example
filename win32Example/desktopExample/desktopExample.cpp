﻿#include "framework.h"
#include "desktopExample.h"
#include <shellapi.h>
#include <vector>
#include <string>
#include <sstream>
#include <commctrl.h>
#include "getOpenFile.h"

#pragma comment (lib, "comctl32.lib")

typedef std::basic_string<TCHAR> tstring;
typedef std::basic_stringstream<TCHAR> tstringstream;

#define MAX_LOADSTRING 100
#define IDC_EDIT 100
#define IDC_BUTTON   101
#define IDC_CHECKBOX 102

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DESKTOPEXAMPLE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DESKTOPEXAMPLE));

    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DESKTOPEXAMPLE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DESKTOPEXAMPLE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   DragAcceptFiles(hWnd, TRUE);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hEdit = nullptr;
    static HWND hwndButton = nullptr;
    static HWND hwndListView = nullptr;

    switch (message)
    {
    case WM_CREATE:
    {
        INITCOMMONCONTROLSEX ic;

        ic.dwSize = sizeof(INITCOMMONCONTROLSEX);
        ic.dwICC = ICC_LISTVIEW_CLASSES;  //リストビュー使用時に指定する
        InitCommonControlsEx(&ic);

        //                                                                            詳細表示のリストビュー(LVS_REPORT)
        hwndListView = CreateWindowEx(0, WC_LISTVIEW, _T(""), WS_CHILD | WS_VISIBLE | LVS_REPORT, 0, 0, 0, 0, hWnd, (HMENU)1, ((LPCREATESTRUCT)lParam)->hInstance, nullptr);

        LVCOLUMN column;
        column.mask = LVCF_WIDTH | LVCF_TEXT;  //csメンバとpszTextメンバを有効化
        column.cx = 100;  //列幅[pixels]
        column.pszText = const_cast<WCHAR*>(L"col1");
        //                    ハンドル      列のidx  列構造体
        ListView_InsertColumn(hwndListView, 0,       &column);

        column.mask = LVCF_WIDTH | LVCF_TEXT;
        column.cx = 100;
        column.pszText = const_cast<WCHAR*>(L"col2");
        ListView_InsertColumn(hwndListView, 1, &column);

        LVITEM item;
        item.mask = LVIF_TEXT;  //pszTextメンバを有効化
        item.iItem = 0;  //エントリーのindex(行のindex)
        item.iSubItem = 0;  //アイテムのindex(列のindex)
        item.pszText = const_cast<WCHAR*>(L"item1");
        ListView_InsertItem(hwndListView, &item);

        item.mask = LVIF_TEXT;
        item.iItem = 0;
        item.iSubItem = 1;
        item.pszText = const_cast<WCHAR*>(L"subitem");
        ListView_SetItem(hwndListView, &item);

        hEdit = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            L"EDIT",
            L"",
            WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL |
            ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE,
            10, 10, 200, 200, 
            hWnd,
            (HMENU)IDC_EDIT, hInst, nullptr);

        hwndButton = CreateWindowEx(0, _T("BUTTON"), _T("ボタン"), WS_CHILD | WS_VISIBLE, 250, 10, 80, 40, hWnd, (HMENU)IDC_BUTTON, ((LPCREATESTRUCT)lParam)->hInstance, nullptr);
        CreateWindowEx(0, _T("BUTTON"), _T("ボタンを有効化する"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 350, 10, 170, 40, hWnd, (HMENU)IDC_CHECKBOX, ((LPCREATESTRUCT)lParam)->hInstance, nullptr);
        CheckDlgButton(hWnd, IDC_CHECKBOX, BST_CHECKED);

        break;
    }
    case WM_SIZE:
    {
        MoveWindow(hwndListView, 0, 300, LOWORD(lParam), HIWORD(lParam), TRUE);
        break;
    }
    case WM_DROPFILES:
    {
        std::shared_ptr<void> hDrop((HDROP)wParam, DragFinish);

        //第二引数0xffffffff ---> ドロップされたファイル数
        int numDropedFiles = DragQueryFile((HDROP)hDrop.get(), -1, nullptr, 0);
        if (numDropedFiles == 0)
        {
            break;
        }

        tstringstream ssFileList;
        for (int i = 0; i < numDropedFiles; i++)
        {
            //  0 < 第二引数 < 合計数 ---> 第三引数がnullの時はファイル名に必要な文字数を返す
            int requiredSize = DragQueryFile((HDROP)hDrop.get(), i, nullptr, 0);
            std::vector<TCHAR> szFileName(requiredSize);
            DragQueryFile((HDROP)hDrop.get(), i, szFileName.data(), szFileName.size() + 1);
            ssFileList << szFileName.data();
            ssFileList << _T("\r\n");
        }
        SetWindowText(hEdit, ssFileList.str().c_str());
        break;
    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDC_BUTTON:
                MessageBox(hWnd, _T("ボタンが押されました。"), _T("OK"), MB_OK);
                break;
            case IDC_CHECKBOX:
                EnableWindow(hwndButton, IsDlgButtonChecked(hWnd, IDC_CHECKBOX) == BST_CHECKED);
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDM_OPEN:
                getOpenFile();
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
