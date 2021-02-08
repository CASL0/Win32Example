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

	//�T�[�r�X����}�l�[�W���[�n���h�����擾����
	SC_HANDLE hSCM = OpenSCManager(
		nullptr,			//�R���s���[�^���̎w��Bnull�̏ꍇ�A���[�J���R���s���[�^
		nullptr,			//DB���̎w��Bnull�̏ꍇ�ASERVICES_ACTIVE_DATABASE
		SC_MANAGER_CONNECT	//https://docs.microsoft.com/en-us/windows/win32/services/service-security-and-access-rights
	);

	if (hSCM == nullptr)
	{
		std::cout << "OpenSCManager failed with error: " << GetLastError() << std::endl;
		return false;
	}

	//SCM --> service handle
	SC_HANDLE hService = OpenService(
		hSCM,				//�T�[�r�X����n���h��
		lpszServiceName,	//�T�[�r�X�����w��(�\�����ł͂Ȃ�)
		READ_CONTROL		//QueryServiceObjectSecurity���Ăяo���ۂɎw�肷��
	);

	if (hService == nullptr)
	{
		std::cout << "OpenService failed with error: " << GetLastError() << std::endl;
		CloseServiceHandle(hSCM);
		return false;
	}

	DWORD dwSdSize;
	//SD�ɕK�v�ȃT�C�Y���擾
	QueryServiceObjectSecurity(hService, DACL_SECURITY_INFORMATION, nullptr, 0, &dwSdSize);

	PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR, dwSdSize);
	if (pSD == nullptr)
	{
		std::cout << "LocalAlloc failed with error: " << GetLastError() << std::endl;
		CloseServiceHandle(hSCM);
		CloseServiceHandle(hService);
		return false;
	}

	//�T�[�r�X --> SD
	QueryServiceObjectSecurity(hService, DACL_SECURITY_INFORMATION, pSD, dwSdSize, &dwSdSize);

	//SD --> DACL
	PACL pDacl;
	BOOL bDaclPresent;
	BOOL bDaclDefaulted;
	GetSecurityDescriptorDacl(
		pSD, 
		&bDaclPresent,		//TRUE --> SD��DACL�����܂�ł���
		&pDacl,				//DACL�����݂���ꍇ�͎擾����
		&bDaclDefaulted		//TRUE --> �f�t�H���g�̃��J�j�Y����DACL���擾
	);

	if (!bDaclPresent)
	{
		std::cout << "SD does not contain DACL" << std::endl;
		LocalFree(pSD);
		CloseServiceHandle(hSCM);
		CloseServiceHandle(hService);
		return false;
	}

	//DACL�̃T�C�Y���擾
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
		//SID����ACE�̃A�J�E���g�A�h���C�������擾����
		LookupAccountSid(
			nullptr,			//null --> ���[�J���}�V��
			&pAce->SidStart,	//�J�nSID
			szAccountName,		//�A�J�E���g��
			&dwAccountNameSize,
			szDomainName,		//�h���C����
			&dwDomainNameSize,
			&snu				//�A�J�E���g�̎��
		);

		std::tcout << "�A�J�E���g��: " << szAccountName << std::endl;
		std::tcout << "�h���C����: " << szDomainName << std::endl;

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