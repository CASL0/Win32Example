#include <Windows.h>
#include <winhttp.h>
#include <memory>
#include <iostream>
#include <vector>
#include "httpRequest.h"
#pragma comment(lib,"winhttp.lib")

bool request()
{
	//HTTP�Z�b�V�����̏ڍׂ�ێ�����n���h��
	std::shared_ptr<std::remove_pointer<HINTERNET>::type> hSession(
		WinHttpOpen(L"WinHTTP Example/1.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0),
		//			UA						IE�v���L�V�ݒ�𓥏P				 �Œ�					�Œ�					 �Œ�	
		WinHttpCloseHandle
	);
	if (hSession == nullptr)
	{
		std::cerr << "WinHttpOpen failed with error: " << GetLastError() << std::endl;
		return false;
	}

	std::wstring sUrl = L"http://eternalwindows.jp/network/winhttp/sec/sample.html";
	URL_COMPONENTS urlComponents = { 0 };
	urlComponents.dwStructSize = sizeof(URL_COMPONENTS);
	std::vector<WCHAR> szHost((DWORD)-1);
	std::vector<WCHAR> szPath((DWORD)-1);
	urlComponents.lpszHostName = szHost.data();
	urlComponents.dwHostNameLength = szHost.size();
	urlComponents.lpszUrlPath = szPath.data();
	urlComponents.dwUrlPathLength = szPath.size();
	bool bRet = WinHttpCrackUrl(sUrl.c_str(), sUrl.length(), 0, &urlComponents);
	if (!bRet)
	{
		std::cerr << "WinHttpCrakUrl failed with error: " << GetLastError() << std::endl;
		return false;
	}

	//�T�[�o�[�̐ݒ�
	std::shared_ptr<std::remove_pointer<HINTERNET>::type> hConnect(
		WinHttpConnect(hSession.get(),
			szHost.data(),				//FQDN or IP�A�h���X������
			INTERNET_DEFAULT_HTTPS_PORT,//�|�[�g�ԍ��B���̃t���O�̏ꍇ�͌��WINHTTP_FLAG_SECURE��ݒ肷��(WinHttpOpenRequest)
										//HTTP�̏ꍇ�́uINTERNET_DEFAULT_HTTP_PORT�v
			0							//0�ŌŒ�
		),
		WinHttpCloseHandle
	);
	if (hConnect == nullptr)
	{
		std::cerr << "WinHttpConnect failed with error: " << GetLastError() << std::endl;
		return false;
	}

	//HTTP���N�G�X�g�n���h���̍쐬
	std::shared_ptr<std::remove_pointer<HINTERNET>::type> hRequest(
		WinHttpOpenRequest(hConnect.get(),
			L"GET",							//���N�G�X�g���\�b�h
			szPath.data(),					//�p�X
			nullptr,						//HTTP�o�[�W�����Bnullptr ---> HTTP1.1
			WINHTTP_NO_REFERER,				//�Q�ƌ�URL�̎w��BWINHTTP_NO_REFERER ---> �Q�ƌ��h�L�������g�̖��w��
			WINHTTP_DEFAULT_ACCEPT_TYPES,	//accept-types�w�b�_�̎w��
			WINHTTP_FLAG_SECURE				//HTTPS�̏ꍇ�͂��̃t���O���w��
											//HTTP�̏ꍇ��0
		),
		WinHttpCloseHandle
	);
	if (hRequest == nullptr)
	{
		std::cerr << "WinHttpOpenRequest failed with error: " << GetLastError() << std::endl;
		return false;
	}

	WinHttpSetCredentials(hRequest.get(), WINHTTP_AUTH_TARGET_SERVER, WINHTTP_AUTH_SCHEME_BASIC, L"user", L"pass", nullptr);

	bRet = WinHttpSendRequest(hRequest.get(), 
		WINHTTP_NO_ADDITIONAL_HEADERS,	//�ǉ��̃w�b�_
		0,								//�ǉ��̃w�b�_�T�C�Y
		WINHTTP_NO_REQUEST_DATA,		//POST���\�b�h���ő���{�f�B
		0,								//��4�����̃T�C�Y
		0,								//content-length�w�b�_�̎w��
		0								//�R�[���o�b�N�ɓn������
	);
	if (!bRet)
	{
		std::cerr << "WinHttpSendRequest failed with error: " << GetLastError() << std::endl;
		return false;
	}

	bRet = WinHttpReceiveResponse(hRequest.get(), nullptr);
	//											  �Œ�

	if (!bRet)
	{
		std::cerr << "WinHttpReceiveResponse failed with error: " << GetLastError() << std::endl;
		return false;
	}

	DWORD dwSize = sizeof(DWORD);
	DWORD dwStatusCode;
	bRet = WinHttpQueryHeaders(hRequest.get(),
		WINHTTP_QUERY_STATUS_CODE |
		WINHTTP_QUERY_FLAG_NUMBER,
		WINHTTP_HEADER_NAME_BY_INDEX,
		&dwStatusCode,
		&dwSize,
		nullptr);
	if (!bRet)
	{
		std::cerr << "WinHttpQueryHeaders failed with error: " << GetLastError() << std::endl;
		return false;
	}
	std::cerr << "Status code: " << dwStatusCode << std::endl;

	WinHttpQueryHeaders(hRequest.get(),
		WINHTTP_QUERY_RAW_HEADERS_CRLF, //CRLF�܂őS�Ă�ǂݍ���
		WINHTTP_HEADER_NAME_BY_INDEX,	//��������WINHTTP_QUERY_CUSTOM�łȂ���΂��̒l
		nullptr,						//�������擾���邽��nullptr�Ƃ���
		&dwSize,						//�T�C�Y�̎擾
		WINHTTP_NO_HEADER_INDEX			//�����w�b�_������Ƃ��ȊOWINHTTP_NO_HEADER_INDEX���w��
	);

	std::vector<BYTE> byteHeader(dwSize);
	bRet = WinHttpQueryHeaders(hRequest.get(), WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, byteHeader.data(), &dwSize, WINHTTP_NO_HEADER_INDEX);
	if (!bRet)
	{
		std::cerr << "WinHttpQueryHeaders failed with error: " << GetLastError() << std::endl;
		return false;
	}

	std::wstring sHeader((LPWSTR)byteHeader.data());
	std::wcerr << sHeader << std::endl;
	std::string sResponse;
	do
	{
		dwSize = 0;
		if (!WinHttpQueryDataAvailable(hRequest.get(), &dwSize))
		{
			std::cerr << "WinHttpQueryDataAvailable failed with error: " << GetLastError() << std::endl;
		}
		std::vector<CHAR> szBuffer(dwSize + 1);
		DWORD dwDownloaded;
		if (!WinHttpReadData(hRequest.get(), (LPVOID)szBuffer.data(), dwSize, &dwDownloaded))
		{
			std::cerr << "WinHttpReadData failed with error: " << GetLastError() << std::endl;
		}
		else
		{
			sResponse += std::string(szBuffer.data());
		}

	} while (dwSize > 0);
	std::cerr << sResponse;
	return true;
}