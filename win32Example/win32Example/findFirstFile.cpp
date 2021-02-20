#include "findFirstFile.h"
#include <Shlwapi.h>
#include <iostream>
#include <memory>

void getAllFileNamesFromDir(std::vector<tstring>& fileNameList, std::vector<TCHAR> szDirPath)
{
	//終端にバックスラッシュを付与(付いている場合はそのまま)
	PathAddBackslash(szDirPath.data());

    //マッチパターンの作成
    std::vector<TCHAR> szPattern(MAX_PATH);
    _tcscpy_s(szPattern.data(), MAX_PATH, szDirPath.data());

	const tstring sMatchPattern = _T("*.*"); //任意のファイルをマッチ
    _tcscat_s(szPattern.data(), MAX_PATH, sMatchPattern.c_str());

	WIN32_FIND_DATA fdFile;
    std::shared_ptr<void> hFind(FindFirstFile(szPattern.data(), &fdFile), FindClose);
	if (hFind.get() == INVALID_HANDLE_VALUE)
	{
        tcout << _T("FindFirstFile failed with error: ") << GetLastError() << std::endl;;
		return;
	}

    do 
    {
        const tstring sFileName = fdFile.cFileName;

        //ディレクトリの場合再帰呼び出し
        if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            //隠しファイルは無視
            if (_tcscmp(sFileName.c_str(), _T(".")) == 0 || _tcscmp(sFileName.c_str(), _T("..")) == 0)
            {
                continue;
            }

            std::vector<TCHAR> tmp(szDirPath);
            _tcscat_s(tmp.data(), MAX_PATH, sFileName.c_str());
            getAllFileNamesFromDir(fileNameList, tmp);

        }
        else
        {
            fileNameList.push_back(sFileName);
        }
    } while (FindNextFile(hFind.get(), &fdFile));

}