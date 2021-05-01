#include <Windows.h>
#include <intshcut.h>
#include <atlbase.h>
#include <iostream>
#include "createWebShortcut.h"

bool CreateShortcut(const tstring& szURL, const tstring& szPath)
{
	HRESULT hr;


	CoInitialize(nullptr);

	{
		CComPtr<IUniformResourceLocator> pUrl;	// �C���X�^���X�p�|�C���^�ϐ�

		// URL �C���X�^���X�쐬
		hr = CoCreateInstance(
			CLSID_InternetShortcut,
			nullptr,
			CLSCTX_INPROC_SERVER,
			IID_IUniformResourceLocator,
			(void**)&pUrl
		);

		if (FAILED(hr))
		{
			std::cout << "CoCreateInstance failed" << std::endl;
			return false;
		}

		// URL �f�[�^�Z�b�g
		hr = pUrl->SetURL(szURL.c_str(), 0);

		if (FAILED(hr))
		{
			std::cout << "SetURL failed" << std::endl;
			return false;
		}

		// �V���[�g�J�b�g�ۑ�����
		CComPtr<IPersistFile> pPf;
		hr = pUrl->QueryInterface(IID_IPersistFile, (void**)&pPf);
		if (SUCCEEDED(hr))
		{
			hr = pPf->Save(szPath.c_str(), TRUE);
		}

	}
	CoUninitialize();
	return true;
}