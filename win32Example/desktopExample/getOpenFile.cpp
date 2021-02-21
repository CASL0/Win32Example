#include <Windows.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <iostream>

typedef std::basic_string<TCHAR> tstring;
#if defined(UNICODE) || defined(_UNICODE)
#define tcout std::wcout
#else
#define tcout std::cout
#endif


bool getOpenFile()
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = nullptr;

	//ファイル名を格納するバッファ
	std::vector<TCHAR> szFilePath(MAX_PATH);
	ofn.lpstrFile = szFilePath.data();
	ofn.nMaxFile = szFilePath.size();
	//初期文字列を指定することができる。
	//不要の場合ヌル文字にする
	ofn.lpstrFile[0] = '\0';

	
	//選択対象とするファイルのフィルター
	//[表示文字列]\0[フィルター文字列]の形式で列挙する
	//例：All Files\0*.*\0 ---> All Filesと表示し全拡張子を対象にする
	ofn.lpstrFilter = _T("すべて\0*.*\0実行ファイル\0*.exe\0");

	//デフォルトのフィルターのインデックス
	//上の例の場合
	//  1 ---> すべて
	//  2 ---> 実行ファイル
	ofn.nFilterIndex = 1;

	//パスからファイル名だけ取り出す
	std::vector<TCHAR> szFileName(MAX_PATH);
	ofn.lpstrFileTitle = szFileName.data();
	ofn.nMaxFileTitle = szFileName.size();

	//初期ディレクトリ
	//nullptrの場合
	//  カレントディレクトリにフィルターに指定したファイルが含まれていればカレントディレクトリ
	//  それ以外 ---> ユーザーディレクトリ
	ofn.lpstrInitialDir = nullptr;

	//OFN_PATHMUSTEXIST ---> 存在しないパスの場合は警告が表示される
	//OFN_FILEMUSTEXIST ---> 存在しないファイルの場合は警告が表示される
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn) != TRUE)
	{
		return false;
	}

	tcout << _T("ファイルパス") << ofn.lpstrFile << std::endl;
	tcout << _T("ファイル名") << ofn.lpstrFileTitle << std::endl;
	return true;
}