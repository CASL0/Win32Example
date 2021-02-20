#include "findFirstFile.h"
#include <Shlwapi.h>
#include <iostream>
#include <memory>

void getAllFileNamesFromDir(std::vector<tstring>& fileNameList, std::vector<TCHAR> szDirPath)
{
	//�I�[�Ƀo�b�N�X���b�V����t�^(�t���Ă���ꍇ�͂��̂܂�)
	PathAddBackslash(szDirPath.data());

    //�}�b�`�p�^�[���̍쐬
    std::vector<TCHAR> szPattern(MAX_PATH);
    _tcscpy_s(szPattern.data(), MAX_PATH, szDirPath.data());

	const tstring sMatchPattern = _T("*.*"); //�C�ӂ̃t�@�C�����}�b�`
    _tcscat_s(szPattern.data(), MAX_PATH, sMatchPattern.c_str());

	WIN32_FIND_DATA fdFile;
    std::shared_ptr<void> hFind(FindFirstFile(szPattern.data(), &fdFile), FindClose);
	if (hFind.get() == INVALID_HANDLE_VALUE)
	{
        tcout << _T("FindFirstFile failed with error: ") << GetLastError() << std::endl;;
		return;
	}

    do 
    {
        const tstring sFileName = fdFile.cFileName;

        //�f�B���N�g���̏ꍇ�ċA�Ăяo��
        if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            //�B���t�@�C���͖���
            if (_tcscmp(sFileName.c_str(), _T(".")) == 0 || _tcscmp(sFileName.c_str(), _T("..")) == 0)
            {
                continue;
            }

            std::vector<TCHAR> tmp(szDirPath);
            _tcscat_s(tmp.data(), MAX_PATH, sFileName.c_str());
            getAllFileNamesFromDir(fileNameList, tmp);

        }
        else
        {
            fileNameList.push_back(sFileName);
        }
    } while (FindNextFile(hFind.get(), &fdFile));

}