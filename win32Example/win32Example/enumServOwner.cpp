#pragma once
#include <Windows.h>
#include <AclAPI.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <memory>
#include "enumServOwner.h"

typedef std::basic_string<TCHAR> tstring;
#if defined(_UNICODE) || defined(UNICODE)
#  define tcout std::wcout
#else
#  define tcout std::cout
#endif

BOOL SIDtoName(PSID pSid, std::vector<TCHAR>& szName);
std::vector<tstring> enumServ();

BOOL SIDtoName(PSID pSid, std::vector<TCHAR>& szName)
{
	DWORD dwSize = szName.size();
	std::vector<TCHAR> szDomainName(256);
	DWORD dwSizeDomain = szDomainName.size();
	SID_NAME_USE sidName;

	BOOL bRet = LookupAccountSid(NULL, pSid, szName.data(), &dwSize, szDomainName.data(), &dwSizeDomain, &sidName);
	if (!bRet)
	{
		std::cerr << "LookupAccountSid failed with error: " << GetLastError() << std::endl;
	}
	return bRet;

}

bool getOwner()
{
	auto servNameList= enumServ();
	for (const auto& elem : servNameList)
	{
		std::vector<TCHAR> szOwnerName(256);
		tstring sServName = elem;
		PSID pSidOwner;
		PSECURITY_DESCRIPTOR pSecurityDescriptor;

		DWORD dwRet = GetNamedSecurityInfo(sServName.c_str(), SE_SERVICE, OWNER_SECURITY_INFORMATION, &pSidOwner, nullptr, nullptr, nullptr, &pSecurityDescriptor);
		if (dwRet != ERROR_SUCCESS)
		{
			std::cerr << "GetNamedSecurityInfo failed with error: " << dwRet << std::endl;
			return false;
		}

		SIDtoName(pSidOwner, szOwnerName);
		tcout << sServName.c_str() << std::endl;
		tcout << _T("\t所有者：") << szOwnerName.data() << std::endl;
		LocalFree(pSecurityDescriptor);

	}
	
	
	return true;
}

std::vector<tstring> enumServ()
{
	std::shared_ptr<std::remove_pointer<SC_HANDLE>::type> hSCM(
		OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE),
		CloseServiceHandle
	);

	DWORD dwBytesNeeded;
	DWORD dwServicesReturned;

	::EnumServicesStatus(hSCM.get(), SERVICE_WIN32 | SERVICE_DRIVER, SERVICE_STATE_ALL, nullptr, 0, &dwBytesNeeded, &dwServicesReturned, 0);

	std::shared_ptr<std::remove_pointer<LPENUM_SERVICE_STATUS>::type> lpEnumServiceStatus(
		(LPENUM_SERVICE_STATUS)LocalAlloc(LPTR, dwBytesNeeded),
		LocalFree
	);

	//サービスの状態を取得する
	EnumServicesStatus(hSCM.get(), SERVICE_WIN32 | SERVICE_DRIVER, SERVICE_STATE_ALL, lpEnumServiceStatus.get(), dwBytesNeeded, &dwBytesNeeded, &dwServicesReturned, 0);

	std::vector<tstring> servNameList;
	for (int i = 0; i < dwServicesReturned; i++)
	{
		//tcout << "ServiceName: " << lpEnumServiceStatus.get()[i].lpServiceName << std::endl;
		servNameList.push_back(lpEnumServiceStatus.get()[i].lpServiceName);
	}
	return servNameList;

}