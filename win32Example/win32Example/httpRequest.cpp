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
	//HTTPセッションの詳細を保持するハンドル
	std::shared_ptr<std::remove_pointer<HINTERNET>::type> hSession(
		WinHttpOpen(L"WinHTTP Example/1.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC),
		//			UA						IEプロキシ設定を踏襲				 固定					固定					 固定	
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

	//サーバーの設定
	std::shared_ptr<std::remove_pointer<HINTERNET>::type> hConnect(
		WinHttpConnect(hSession.get(),
			szHost.data(),				//FQDN or IPアドレス文字列
			INTERNET_DEFAULT_HTTPS_PORT,//ポート番号。このフラグの場合は後でWINHTTP_FLAG_SECUREを設定する(WinHttpOpenRequest)
										//HTTPの場合は「INTERNET_DEFAULT_HTTP_PORT」
			0							//0で固定
		),
		WinHttpCloseHandle
	);
	if (hConnect == nullptr)
	{
		std::cerr << "WinHttpConnect failed with error: " << GetLastError() << std::endl;
		return false;
	}

	//HTTPリクエストハンドルの作成
	std::shared_ptr<std::remove_pointer<HINTERNET>::type> hRequest(
		WinHttpOpenRequest(hConnect.get(),
			L"GET",							//リクエストメソッド
			szPath.data(),					//パス
			nullptr,						//HTTPバージョン。nullptr ---> HTTP1.1
			WINHTTP_NO_REFERER,				//参照元URLの指定。WINHTTP_NO_REFERER ---> 参照元ドキュメントの未指定
			WINHTTP_DEFAULT_ACCEPT_TYPES,	//accept-typesヘッダの指定
			WINHTTP_FLAG_SECURE				//HTTPSの場合はこのフラグを指定
											//HTTPの場合は0
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
		WINHTTP_NO_ADDITIONAL_HEADERS,	//追加のヘッダ
		0,								//追加のヘッダサイズ
		WINHTTP_NO_REQUEST_DATA,		//POSTメソッド等で送るボディ
		0,								//第4引数のサイズ
		0,								//content-lengthヘッダの指定
		(DWORD_PTR)nullptr				//コールバックに渡す引数
	);
	if (!bRet)
	{
		std::cerr << "WinHttpSendRequest failed with error: " << GetLastError() << std::endl;
		return false;
	}

	MessageBox(nullptr, TEXT("ボタンを押すと終了します。"), TEXT("OK"), MB_OK);

	/*
	bRet = WinHttpReceiveResponse(hRequest.get(), nullptr);
	//											  固定

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
		WINHTTP_QUERY_RAW_HEADERS_CRLF, //CRLFまで全てを読み込む
		WINHTTP_HEADER_NAME_BY_INDEX,	//第二引数がWINHTTP_QUERY_CUSTOMでなければこの値
		nullptr,						//長さを取得するためnullptrとする
		&dwSize,						//サイズの取得
		WINHTTP_NO_HEADER_INDEX			//複数ヘッダがあるとき以外WINHTTP_NO_HEADER_INDEXを指定
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
		sHostName.c_str(),		//資格情報のターゲット名(サーバー等)
		nullptr,				//nullptrで固定
		0, 
		szTmpUserName.data(),	//「ドメイン\ユーザー名」で格納される
		szTmpUserName.size(), 
		szPasswd.data(),		//パスワードの格納
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
		szTmpUserName.data(), //UPN等のアカウント名
		szUserName.data(),	  //ユーザー名を抽出し格納する
		szUserName.size(), 
		szDomain.data(),	  //ドメイン
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

	//リクエストが完了
	//lpvStatusInformationはnullptr
	case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
		bRet = WinHttpReceiveResponse(hRequest, nullptr);
		if (!bRet)
		{
			std::cerr << "WinHttpReceiveResponse failed with error: " << GetLastError() << std::endl;
			return;
		}
		break;

	//応答ヘッダを受信済み、WinHttpQueryHeadersで問い合わせること可能
	//lpvStatusInformationはnullptr
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

	//WinHttpReadDataを使ってデータが読み取り可能
	//lpvStatusInformationは利用可能データのバイト数
	//dwStatusInformationLengthは4(DWORDのサイズ)
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
		//非同期の場合は第二引数は必ずnullptrにする
		//「非同期」かつ「返り値がTRUE」かつ「データがavailable」--> WINHTTP_STATUS_CALLBACK_DATA_AVAILABLE
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