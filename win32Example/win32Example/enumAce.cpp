#include <AclAPI.h>
#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <winerror.h>

#pragma comment(lib, "Advapi32.lib")

int enumAce(const std::string& sDirectoryPath)
{
	DWORD dwRet;
	PSECURITY_DESCRIPTOR pSD;
	PACL pDacl;
	//セキュリティディスクリプタとDACLの取得
	dwRet = GetNamedSecurityInfoA(sDirectoryPath.c_str(),
		SE_FILE_OBJECT,	//ファイルまたはディレクトリ
		DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
		nullptr,	//所有者SID
		nullptr,	//グループSID
		&pDacl,		//DACL
		nullptr,	//SACL
		&pSD		//セキュリティディスクリプタ
	);
	if (dwRet != ERROR_SUCCESS)
	{
		std::cout << "GetNamedSecurityInfoA failed with error: " << dwRet << std::endl;
		return 1;
	}

	ACL_SIZE_INFORMATION aclSize;
	bool bRet;
	//DACLのサイズを取得する
	bRet = GetAclInformation(pDacl, &aclSize, sizeof(aclSize), AclSizeInformation);
	if (!bRet)
	{
		std::cout << "GetAclInformation failed with error: " << GetLastError() << std::endl;
		return 1;
	}

	ACCESS_ALLOWED_ACE* pAce;
	for (int i = 0; i < aclSize.AceCount; i++)
	{
		bRet = GetAce(pDacl, i, (LPVOID*)&pAce);
		if (!bRet)
		{
			std::cout << "GetAce failed with error: " << GetLastError() << std::endl;
			return 1;
		}

		DWORD dwAccountNameSize;
		std::vector<CHAR> vecAccountName(256);

		DWORD dwDomainNameSize;
		std::vector<CHAR> vecDomainName(256);

		SID_NAME_USE snu;
		LookupAccountSidA(
			nullptr,				//システム名(リモートコンピュータの場合のみ指定), nullptrの場合はローカルシステム
			&pAce->SidStart,
			vecAccountName.data(),
			&dwAccountNameSize,
			vecDomainName.data(),
			&dwDomainNameSize,
			&snu					//SIDの種別
		);

		std::cout << "AccountName: " << vecAccountName.data() << std::endl << "vecDomainName: " << vecDomainName.data() << std::endl;

		//アクセス権の列挙
		std::cout << "AccessMask" << std::endl;
		if (pAce->Mask & FILE_LIST_DIRECTORY)
		{
			std::cout << "\tFILE_LIST_DIRECTORY" << std::endl;
		}
		if (pAce->Mask & FILE_ADD_FILE)
		{
			std::cout << "\tFILE_ADD_FILE" << std::endl;
		}
		if (pAce->Mask & FILE_ADD_SUBDIRECTORY)
		{
			std::cout << "\tFILE_ADD_SUBDIRECTORY" << std::endl;
		}
		if (pAce->Mask & FILE_READ_EA)
		{
			std::cout << "\tFILE_READ_EA" << std::endl;
		}
		if (pAce->Mask & FILE_WRITE_EA)
		{
			std::cout << "\tFILE_WRITE_EA" << std::endl;
		}
		if (pAce->Mask & FILE_TRAVERSE)
		{
			std::cout << "\tFILE_TRAVERSE" << std::endl;
		}
		if (pAce->Mask & FILE_DELETE_CHILD)
		{
			std::cout << "\tFILE_DELETE_CHILD" << std::endl;
		}
		if (pAce->Mask & FILE_READ_ATTRIBUTES)
		{
			std::cout << "\tFILE_READ_ATTRIBUTES" << std::endl;
		}
		if (pAce->Mask & FILE_WRITE_ATTRIBUTES)
		{
			std::cout << "\tFILE_WRITE_ATTRIBUTES" << std::endl;
		}
		if (pAce->Mask & FILE_ALL_ACCESS)
		{
			std::cout << "\tFILE_ALL_ACCESS" << std::endl;
		}
		if (pAce->Mask & FILE_GENERIC_READ)
		{
			std::cout << "\tFILE_GENERIC_READ" << std::endl;
		}
		if (pAce->Mask & FILE_GENERIC_WRITE)
		{
			std::cout << "\tFILE_GENERIC_WRITE" << std::endl;
		}
		if (pAce->Mask & FILE_GENERIC_EXECUTE)
		{
			std::cout << "\tFILE_GENERIC_EXECUTE" << std::endl;
		}

		//標準のアクセス権の列挙
		std::cout << "Standard Access Rights" << std::endl;
		if (pAce->Mask & DELETE)
		{
			std::cout << "\tDELETE" << std::endl;
		}
		if (pAce->Mask & READ_CONTROL)
		{
			std::cout << "\tREAD_CONTROL" << std::endl;
		}
		if (pAce->Mask & WRITE_DAC)
		{
			std::cout << "\tWRITE_DAC" << std::endl;
		}
		if (pAce->Mask & WRITE_OWNER)
		{
			std::cout << "\tWRITE_OWNER" << std::endl;
		}
		if (pAce->Mask & SYNCHRONIZE)
		{
			std::cout << "\tSYNCHRONIZE" << std::endl;
		}
		if (pAce->Mask & STANDARD_RIGHTS_REQUIRED)
		{
			std::cout << "\tSTANDARD_RIGHTS_REQUIRED" << std::endl;
		}
		if (pAce->Mask & STANDARD_RIGHTS_READ)
		{
			std::cout << "\tSTANDARD_RIGHTS_READ" << std::endl;
		}
		if (pAce->Mask & STANDARD_RIGHTS_WRITE)
		{
			std::cout << "\tSTANDARD_RIGHTS_WRITE" << std::endl;
		}
		if (pAce->Mask & STANDARD_RIGHTS_EXECUTE)
		{
			std::cout << "\tSTANDARD_RIGHTS_EXECUTE" << std::endl;
		}
		if (pAce->Mask & STANDARD_RIGHTS_ALL)
		{
			std::cout << "\tSTANDARD_RIGHTS_ALL" << std::endl;
		}
		if (pAce->Mask & SPECIFIC_RIGHTS_ALL)
		{
			if (pAce->Mask & SPECIFIC_RIGHTS_ALL)
				std::cout << "\tSPECIFIC_RIGHTS_ALL" << std::endl;
		}

		//ACEの種別
		std::cout << "ACE type" << std::endl;
		switch (pAce->Header.AceType)
		{
		case ACCESS_ALLOWED_ACE_TYPE:
			std::cout << "\tACCESS_ALLOWED_ACE_TYPE" << std::endl;
			break;
		case ACCESS_DENIED_ACE_TYPE:
			std::cout << "\tACCESS_DENIED_ACE_TYPE" << std::endl;
			break;
		case SYSTEM_AUDIT_ACE_TYPE:
			std::cout << "\tSYSTEM_AUDIT_ACE_TYPE" << std::endl;
			break;
		default:
			std::cout << "\tUnknown ACE type" << std::endl;
			break;
		}
	}
	LocalFree(pSD);
	return 0;
}