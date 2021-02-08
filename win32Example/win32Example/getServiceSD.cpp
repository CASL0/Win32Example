#include "service.h"
#include <Windows.h>
#include <iostream>
#include <tchar.h>

namespace std
{
#ifdef UNICODE
	wostream& tcout = wcout;
#else
	ostream& tcout = cout;
#endif // UNICODE
}

bool GetServiceSD(LPCTSTR lpszServiceName)
{

	//サービス制御マネージャーハンドルを取得する
	SC_HANDLE hSCM = OpenSCManager(
		nullptr,			//コンピュータ名の指定。nullの場合、ローカルコンピュータ
		nullptr,			//DB名の指定。nullの場合、SERVICES_ACTIVE_DATABASE
		SC_MANAGER_CONNECT	//https://docs.microsoft.com/en-us/windows/win32/services/service-security-and-access-rights
	);

	if (hSCM == nullptr)
	{
		std::cout << "OpenSCManager failed with error: " << GetLastError() << std::endl;
		return false;
	}

	//SCM --> service handle
	SC_HANDLE hService = OpenService(
		hSCM,				//サービス制御ハンドル
		lpszServiceName,	//サービス名を指定(表示名ではない)
		READ_CONTROL		//QueryServiceObjectSecurityを呼び出す際に指定する
	);

	if (hService == nullptr)
	{
		std::cout << "OpenService failed with error: " << GetLastError() << std::endl;
		CloseServiceHandle(hSCM);
		return false;
	}

	DWORD dwSdSize;
	//SDに必要なサイズを取得
	QueryServiceObjectSecurity(hService, DACL_SECURITY_INFORMATION, nullptr, 0, &dwSdSize);

	PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwSdSize);
	if (pSD == nullptr)
	{
		std::cout << "LocalAlloc failed with error: " << GetLastError() << std::endl;
		CloseServiceHandle(hSCM);
		CloseServiceHandle(hService);
		return false;
	}

	//サービス --> SD
	QueryServiceObjectSecurity(hService, DACL_SECURITY_INFORMATION, pSD, dwSdSize, &dwSdSize);

	//SD --> DACL
	PACL pDacl;
	BOOL bDaclPresent;
	BOOL bDaclDefaulted;
	GetSecurityDescriptorDacl(
		pSD, 
		&bDaclPresent,		//TRUE --> SDがDACLをを含んでいる
		&pDacl,				//DACLが存在する場合は取得する
		&bDaclDefaulted		//TRUE --> デフォルトのメカニズムでDACLを取得
	);

	if (!bDaclPresent)
	{
		std::cout << "SD does not contain DACL" << std::endl;
		LocalFree(pSD);
		CloseServiceHandle(hSCM);
		CloseServiceHandle(hService);
		return false;
	}

	//DACLのサイズを取得
	ACL_SIZE_INFORMATION aclSize;
	GetAclInformation(pDacl, &aclSize, sizeof(aclSize), AclSizeInformation);

	ACCESS_ALLOWED_ACE* pAce;
	TCHAR szAccountName[256];
	TCHAR szDomainName[256];
	for (int i = 0; i < aclSize.AceCount; i++)
	{
		GetAce(pDacl, i, (LPVOID*)&pAce);

		DWORD dwAccountNameSize = sizeof(szAccountName) / sizeof(szAccountName[0]);
		DWORD dwDomainNameSize = sizeof(szDomainName) / sizeof(szDomainName[0]);

		SID_NAME_USE snu;
		//SIDからACEのアカウント、ドメイン名を取得する
		LookupAccountSid(
			nullptr,			//null --> ローカルマシン
			&pAce->SidStart,	//開始SID
			szAccountName,		//アカウント名
			&dwAccountNameSize,
			szDomainName,		//ドメイン名
			&dwDomainNameSize,
			&snu				//アカウントの種別
		);

		std::tcout << "アカウント名: " << szAccountName << std::endl;
		std::tcout << "ドメイン名: " << szDomainName << std::endl;

		std::tcout << "AccessMask" << std::endl;
		if (pAce->Mask & SERVICE_QUERY_CONFIG)
		{
			std::tcout << "\tSERVICE_QUERY_CONFIG" << std::endl;;
		}
			
		if (pAce->Mask & SERVICE_CHANGE_CONFIG)
		{
			std::tcout << "\tSERVICE_CHANGE_CONFIG" << std::endl;
		}
		if (pAce->Mask & SERVICE_QUERY_STATUS)
		{
			std::tcout << "\tSERVICE_QUERY_STATUS" << std::endl;
		}
		if (pAce->Mask & SERVICE_START)
		{
			std::tcout << "\tSERVICE_START" << std::endl;
		}
		if (pAce->Mask & SERVICE_STOP)
		{
			std::tcout << "\tSERVICE_STOP" << std::endl;
		}
		if (pAce->Mask & SERVICE_PAUSE_CONTINUE)
		{
			std::tcout << "\tSERVICE_PAUSE_CONTINUE" << std::endl;
		}
		if (pAce->Mask & SERVICE_INTERROGATE)
		{
			std::tcout << "\tSERVICE_INTERROGATE" << std::endl;
		}
		if (pAce->Mask & SERVICE_USER_DEFINED_CONTROL)
		{
			std::tcout << "\tSERVICE_USER_DEFINED_CONTROL" << std::endl;
		}		
	}
	
	LocalFree(pSD);
	CloseServiceHandle(hSCM);
	CloseServiceHandle(hService);
	return true;
}