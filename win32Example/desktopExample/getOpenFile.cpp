#include <Windows.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <iostream>

typedef std::basic_string<TCHAR> tstring;
#if defined(UNICODE) || defined(_UNICODE)
#define tcout std::wcout
#else
#define tcout std::cout
#endif


bool getOpenFile()
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = nullptr;

	//�t�@�C�������i�[����o�b�t�@
	std::vector<TCHAR> szFilePath(MAX_PATH);
	ofn.lpstrFile = szFilePath.data();
	ofn.nMaxFile = szFilePath.size();
	//������������w�肷�邱�Ƃ��ł���B
	//�s�v�̏ꍇ�k�������ɂ���
	ofn.lpstrFile[0] = '\0';

	
	//�I��ΏۂƂ���t�@�C���̃t�B���^�[
	//[�\��������]\0[�t�B���^�[������]�̌`���ŗ񋓂���
	//��FAll Files\0*.*\0 ---> All Files�ƕ\�����S�g���q��Ώۂɂ���
	ofn.lpstrFilter = _T("���ׂ�\0*.*\0���s�t�@�C��\0*.exe\0");

	//�f�t�H���g�̃t�B���^�[�̃C���f�b�N�X
	//��̗�̏ꍇ
	//  1 ---> ���ׂ�
	//  2 ---> ���s�t�@�C��
	ofn.nFilterIndex = 1;

	//�p�X����t�@�C�����������o��
	std::vector<TCHAR> szFileName(MAX_PATH);
	ofn.lpstrFileTitle = szFileName.data();
	ofn.nMaxFileTitle = szFileName.size();

	//�����f�B���N�g��
	//nullptr�̏ꍇ
	//  �J�����g�f�B���N�g���Ƀt�B���^�[�Ɏw�肵���t�@�C�����܂܂�Ă���΃J�����g�f�B���N�g��
	//  ����ȊO ---> ���[�U�[�f�B���N�g��
	ofn.lpstrInitialDir = nullptr;

	//OFN_PATHMUSTEXIST ---> ���݂��Ȃ��p�X�̏ꍇ�͌x�����\�������
	//OFN_FILEMUSTEXIST ---> ���݂��Ȃ��t�@�C���̏ꍇ�͌x�����\�������
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn) != TRUE)
	{
		return false;
	}

	tcout << _T("�t�@�C���p�X") << ofn.lpstrFile << std::endl;
	tcout << _T("�t�@�C����") << ofn.lpstrFileTitle << std::endl;
	return true;
}