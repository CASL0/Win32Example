#include <Windows.h>
#include <winhttp.h>
#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <wincred.h>
#include "httpRequest.h"
#pragma comment(lib,"winhttp.lib")
#pragma comment (lib, "credui.lib")

void CALLBACK WinHttpStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);
BOOL ShowCredentialDialog(const std::wstring& sHostName, std::wstring& sUserName, std::wstring& sPasswd);

bool request()
{
	//HTTP�Z�b�V�����̏ڍׂ�ێ�����n���h��
	std::shared_ptr<std::remove_pointer<HINTERNET>::type> hSession(
		WinHttpOpen(L"WinHTTP Example/1.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC),
		//			UA						IE�v���L�V�ݒ�𓥏P				 �Œ�					�Œ�					 �Œ�	
		WinHttpCloseHandle
	);

	if (hSession == nullptr)
	{
		std::cerr << "WinHttpOpen failed with error: " << GetLastError() << std::endl;
		return false;
	}

	std::wstring sUrl = L"https://www.microsoft.com";
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
	WinHttpSetStatusCallback(hRequest.get(), WinHttpStatusCallback, WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS, 0);
	bRet = WinHttpSendRequest(hRequest.get(), 
		WINHTTP_NO_ADDITIONAL_HEADERS,	//�ǉ��̃w�b�_
		0,								//�ǉ��̃w�b�_�T�C�Y
		WINHTTP_NO_REQUEST_DATA,		//POST���\�b�h���ő���{�f�B
		0,								//��4�����̃T�C�Y
		0,								//content-length�w�b�_�̎w��
		(DWORD_PTR)nullptr				//�R�[���o�b�N�ɓn������
	);
	if (!bRet)
	{
		std::cerr << "WinHttpSendRequest failed with error: " << GetLastError() << std::endl;
		return false;
	}

	MessageBox(nullptr, TEXT("�{�^���������ƏI�����܂��B"), TEXT("OK"), MB_OK);

	/*
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

	switch (dwStatusCode)
	{
	case HTTP_STATUS_OK:
		return true;
	case HTTP_STATUS_DENIED://401
		break;
	default:
		return false;
	}

	DWORD dwSupportedSchemes, dwFirstScheme, dwAuthTarget;
	WinHttpQueryAuthSchemes(hRequest.get(), &dwSupportedSchemes, &dwFirstScheme, &dwAuthTarget);
	DWORD dwAuthScheme;
	if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_NEGOTIATE)
		dwAuthScheme = WINHTTP_AUTH_SCHEME_NEGOTIATE;
	else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_NTLM)
		dwAuthScheme = WINHTTP_AUTH_SCHEME_NTLM;
	else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_PASSPORT)
		dwAuthScheme = WINHTTP_AUTH_SCHEME_PASSPORT;
	else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_DIGEST)
		dwAuthScheme = WINHTTP_AUTH_SCHEME_DIGEST;
	else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_BASIC)
		dwAuthScheme = WINHTTP_AUTH_SCHEME_BASIC;
	else
		return FALSE;

	std::wstring sUserName;
	std::wstring sPasswd;

	if (!ShowCredentialDialog(szHost.data(), sUserName, sPasswd))
	{
		return FALSE;
	}
	WinHttpSetCredentials(hRequest.get(), WINHTTP_AUTH_TARGET_SERVER, dwAuthScheme, sUserName.c_str(), sPasswd.c_str(), nullptr);
	WinHttpSendRequest(hRequest.get(), WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
	WinHttpReceiveResponse(hRequest.get(), nullptr);
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
	return TRUE;

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
	*/
	return true;
}

BOOL ShowCredentialDialog(const std::wstring& sHostName, std::wstring& sUserName, std::wstring& sPasswd)
{
	constexpr size_t BUFFER_SIZE = 256;
	std::vector<WCHAR> szTmpUserName(BUFFER_SIZE);
	std::vector<WCHAR> szDomain(BUFFER_SIZE);
	std::vector<WCHAR> szPasswd(BUFFER_SIZE);
	DWORD dwRet = CredUIPromptForCredentials(
		nullptr, 
		sHostName.c_str(),		//���i���̃^�[�Q�b�g��(�T�[�o�[��)
		nullptr,				//nullptr�ŌŒ�
		0, 
		szTmpUserName.data(),	//�u�h���C��\���[�U�[���v�Ŋi�[�����
		szTmpUserName.size(), 
		szPasswd.data(),		//�p�X���[�h�̊i�[
		szPasswd.size(), 
		nullptr, 
		CREDUI_FLAGS_DO_NOT_PERSIST
	);
	if (dwRet != NO_ERROR)
	{
		return FALSE;
	}
	sPasswd = std::wstring(szPasswd.data());
	std::vector<WCHAR> szUserName(BUFFER_SIZE);
	dwRet = CredUIParseUserName(
		szTmpUserName.data(), //UPN���̃A�J�E���g��
		szUserName.data(),	  //���[�U�[���𒊏o���i�[����
		szUserName.size(), 
		szDomain.data(),	  //�h���C��
		szDomain.size()
	);
	if (dwRet != NO_ERROR)
	{
		return FALSE;
	}
	sUserName = std::wstring(szUserName.data());
	return TRUE;
}


void CALLBACK WinHttpStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
	HINTERNET hRequest = hInternet;
	BOOL bRet;
	switch (dwInternetStatus)
	{

	//���N�G�X�g������
	//lpvStatusInformation��nullptr
	case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
		bRet = WinHttpReceiveResponse(hRequest, nullptr);
		if (!bRet)
		{
			std::cerr << "WinHttpReceiveResponse failed with error: " << GetLastError() << std::endl;
			return;
		}
		break;

	//�����w�b�_����M�ς݁AWinHttpQueryHeaders�Ŗ₢���킹�邱�Ɖ\
	//lpvStatusInformation��nullptr
	case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
	{
		DWORD dwStatusCode = 0;
		DWORD dwSize = sizeof(DWORD);
		bRet = WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &dwStatusCode, &dwSize, WINHTTP_NO_HEADER_INDEX);
		if (!bRet)
		{
			std::cerr << "WinHttpQueryHeaders failed with error: " << GetLastError() << std::endl;
			return;
		}
		switch (dwStatusCode)
		{
		case HTTP_STATUS_OK:
			if (!WinHttpQueryDataAvailable(hRequest, nullptr))
			{
				std::cerr << "WinHttpQueryDataAvailable failed with error: " << GetLastError() << std::endl;
				return;
			}
			break;
		case HTTP_STATUS_DENIED:
			break;
		default:
		{
			std::wstring sMsg = L"Status Code: ";
			sMsg += std::to_wstring(dwStatusCode);
			MessageBox(nullptr, sMsg.c_str(), nullptr, MB_ICONWARNING);
			break;
		}

		}
	}
	break;

	//WinHttpReadData���g���ăf�[�^���ǂݎ��\
	//lpvStatusInformation�͗��p�\�f�[�^�̃o�C�g��
	//dwStatusInformationLength��4(DWORD�̃T�C�Y)
	case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:
	{
		DWORD dwSize = *((LPDWORD)lpvStatusInformation);
		if (dwSize == 0)
		{
			return;
		}
		std::vector<char> szBuf(dwSize + 1);
		WinHttpReadData(hRequest, szBuf.data(), dwSize, nullptr);
		std::string sResponse(szBuf.data());
		std::cerr << sResponse << std::endl;
	}
	break;
	case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
		//�񓯊��̏ꍇ�͑������͕K��nullptr�ɂ���
		//�u�񓯊��v���u�Ԃ�l��TRUE�v���u�f�[�^��available�v--> WINHTTP_STATUS_CALLBACK_DATA_AVAILABLE
		if (!WinHttpQueryDataAvailable(hRequest, nullptr))
		{
			std::cerr << "WinHttpQueryDataAvailable failed with error: " << GetLastError() << std::endl;
			return;
		}
		break;
	case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
	{
		LPWINHTTP_ASYNC_RESULT lpAsyncResult = (LPWINHTTP_ASYNC_RESULT)lpvStatusInformation;
		std::cerr << "Error\nID: " << lpAsyncResult->dwResult << std::endl << "ret value: " << lpAsyncResult->dwError << std::endl;
	}
	}
}