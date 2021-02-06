#include <AclAPI.h>
#include <vector>
#include <string>
#include <iostream>

#pragma comment(lib, "Advapi32.lib")

static bool GetSidByAccountName(const std::string& sAccountName,
    PSID* ppSid,
    char* lpszDomainName,
    DWORD dwDomainName);

int SetAce()
{
    SECURITY_DESCRIPTOR sd;

    //セキュリティディスクリプタの初期化
    bool bRet = InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    if (!bRet)
    {
        std::cout << "InitializeSecurityDescriptor failed with error: " << GetLastError() << std::endl;
        return 1;
    }

    PACL pDacl;
    constexpr DWORD dwAclSize = 1024;
    pDacl = (PACL)LocalAlloc(LPTR, dwAclSize);
    if (pDacl == nullptr)
    {
        std::cout << "LocalAlloc failed" << std::endl;
        return 1;
    }


    bRet = InitializeAcl(pDacl, dwAclSize, ACL_REVISION);
    if (!bRet)
    {
        LocalFree(pDacl);
        std::cout << "InitializeAcl failed with error: " << GetLastError() << std::endl;
        return 1;
    }

    PSID pSid;
    std::vector<CHAR> vecDomainName(512);
    //SIDの取得
    bRet = GetSidByAccountName("SYSTEM", &pSid, vecDomainName.data(), vecDomainName.size());
    if (!bRet)
    {
        LocalFree(pDacl);
        return 1;
    }
    //ACEをDACLに追加

    //読み取り属性
    AddAccessAllowedAce(pDacl, ACL_REVISION, FILE_GENERIC_READ, pSid);
    //書き込み属性
    AddAccessAllowedAce(pDacl, ACL_REVISION, FILE_GENERIC_WRITE, pSid);

    //セキュリティディスクリプタにDACLの設定をする
    bRet = SetSecurityDescriptorDacl(&sd, TRUE, pDacl, FALSE);
    if (!bRet)
    {
        LocalFree(pDacl);
        std::cout << "SetSecurityDescriptorDacl failed with error: " << GetLastError() << std::endl;
        return 1;
    }

    //ファイルにセキュリティディスクリプタを設定する
    bRet = SetFileSecurityA("test.txt", DACL_SECURITY_INFORMATION, &sd);
    if (!bRet)
    {
        LocalFree(pDacl);
        std::cout << "SetFileSecurityA failed with error: " << GetLastError() << std::endl;
        return 1;
    }
    LocalFree(pSid);
    LocalFree(pDacl);
    
    return 0;
}

static bool GetSidByAccountName(const std::string& sAccountName,
    PSID* ppSid,
    char* lpszDomainName,
    DWORD dwDomainName)
{
    DWORD dwSidSize = 0;
    SID_NAME_USE snu;
    bool bRet;

    //SIDを格納するために、dwSidSizeに必要なバッファサイズを取得する
    bRet = LookupAccountNameA(
        nullptr,                //nullptrの場合ローカルシステム
        sAccountName.c_str(),
        nullptr,
        &dwSidSize,             //0の場合必要なバッファーサイズを取得する
        lpszDomainName,
        &dwDomainName,
        &snu
    );

    //0を指定するとERROR_INSUFFICIENT_BUFFERが返ってくるためそれは除外する
    if (!bRet && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
    {
        std::cout << "LookupAccountName failed with error: " << GetLastError() << std::endl;
        return false;
    }

    *ppSid = (PSID)LocalAlloc(LPTR, dwSidSize);
    if (*ppSid == nullptr) 
    {
        std::cout << "LocalAlloc failed with error: " << GetLastError() << std::endl;
        return false;
    }

    bRet = LookupAccountNameA(
        nullptr,
        sAccountName.c_str(),
        *ppSid,                 //SIDの取得
        &dwSidSize,
        lpszDomainName,
        &dwDomainName,
        &snu
    );

    return bRet;
}