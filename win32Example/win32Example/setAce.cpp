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

    //�Z�L�����e�B�f�B�X�N���v�^�̏�����
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
    //SID�̎擾
    bRet = GetSidByAccountName("SYSTEM", &pSid, vecDomainName.data(), vecDomainName.size());
    if (!bRet)
    {
        LocalFree(pDacl);
        return 1;
    }
    //ACE��DACL�ɒǉ�

    //�ǂݎ�葮��
    AddAccessAllowedAce(pDacl, ACL_REVISION, FILE_GENERIC_READ, pSid);
    //�������ݑ���
    AddAccessAllowedAce(pDacl, ACL_REVISION, FILE_GENERIC_WRITE, pSid);

    //�Z�L�����e�B�f�B�X�N���v�^��DACL�̐ݒ������
    bRet = SetSecurityDescriptorDacl(&sd, TRUE, pDacl, FALSE);
    if (!bRet)
    {
        LocalFree(pDacl);
        std::cout << "SetSecurityDescriptorDacl failed with error: " << GetLastError() << std::endl;
        return 1;
    }

    //�t�@�C���ɃZ�L�����e�B�f�B�X�N���v�^��ݒ肷��
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

    //SID���i�[���邽�߂ɁAdwSidSize�ɕK�v�ȃo�b�t�@�T�C�Y���擾����
    bRet = LookupAccountNameA(
        nullptr,                //nullptr�̏ꍇ���[�J���V�X�e��
        sAccountName.c_str(),
        nullptr,
        &dwSidSize,             //0�̏ꍇ�K�v�ȃo�b�t�@�[�T�C�Y���擾����
        lpszDomainName,
        &dwDomainName,
        &snu
    );

    //0���w�肷���ERROR_INSUFFICIENT_BUFFER���Ԃ��Ă��邽�߂���͏��O����
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
        *ppSid,                 //SID�̎擾
        &dwSidSize,
        lpszDomainName,
        &dwDomainName,
        &snu
    );

    return bRet;
}