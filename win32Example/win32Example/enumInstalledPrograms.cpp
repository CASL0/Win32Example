#include <Windows.h>
#include <tchar.h>
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include "enumUninstalledPrograms.h"

using tstring = std::basic_string<TCHAR>;

bool enumInstalledPrograms()
{
	tstring sRoot = _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
	HKEY hUninstKey = nullptr;
	LSTATUS lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sRoot.c_str(), 0, KEY_READ, &hUninstKey);
	if (lResult != ERROR_SUCCESS)
	{
		return false;
	}

	HKEY hAppKey = nullptr;
	std::vector<TCHAR> szAppKeyName(1024);
	std::vector<TCHAR> szDisplayName(1024);
	LSTATUS lEnumResult = ERROR_SUCCESS;
	for (DWORD dwIndex = 0; lEnumResult == ERROR_SUCCESS; dwIndex++)
	{
		DWORD dwBufferSize = szAppKeyName.size();
		lEnumResult = RegEnumKeyEx(hUninstKey, dwIndex, szAppKeyName.data(), &dwBufferSize, nullptr, nullptr, nullptr, nullptr);
		if (lEnumResult != ERROR_SUCCESS)
		{
			continue;
		}

		tstring sSubKey = sRoot + _T("\\") + tstring(szAppKeyName.data());
		lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sSubKey.c_str(), 0, KEY_READ, &hAppKey);
		if (lResult != ERROR_SUCCESS)
		{
			RegCloseKey(hAppKey);
			RegCloseKey(hUninstKey);
			return false;
		}

		dwBufferSize = szDisplayName.size();
		DWORD dwType = KEY_ALL_ACCESS;
		lResult = RegQueryValueEx(hAppKey, _T("DisplayName"), nullptr, &dwType, (BYTE*)szDisplayName.data(), &dwBufferSize);
		if (lResult == ERROR_SUCCESS)
		{
			std::wcout << szDisplayName.data() << std::endl;
		}
		RegCloseKey(hAppKey);
	}

	RegCloseKey(hUninstKey);
	return true;
}