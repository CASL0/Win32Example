#pragma once
#include <tchar.h>
#include <string>

using tstring = std::basic_string<TCHAR>;
bool CreateShortcut(const tstring& szURL, const tstring& szPath);