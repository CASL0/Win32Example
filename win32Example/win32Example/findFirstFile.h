#pragma once
#include <string>
#include <tchar.h>
#include <vector>

typedef std::basic_string<TCHAR> tstring;
#if defined(UNICODE) || defined(_UNICODE)
#define tcout std::wcout
#else
#define tcout std::cout
#endif

void getAllFileNamesFromDir(std::vector<tstring>& fileNameList, std::vector<TCHAR> szDirPath);