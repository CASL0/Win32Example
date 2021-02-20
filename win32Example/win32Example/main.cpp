#include <tchar.h>
#include <Shlwapi.h>
#include <iostream>
#include "findFirstFile.h"

int _tmain(int argc, _TCHAR* argv[])
{
	std::vector<tstring> fileNameList;
	std::vector<TCHAR> szDirPath(MAX_PATH);
	_tcscpy_s(szDirPath.data(), MAX_PATH, _T("C:\\Windows\\System32"));
	getAllFileNamesFromDir(fileNameList, szDirPath);
	for (const auto& elem : fileNameList)
	{
		tcout << elem << std::endl;
	}
	return 0;
}