#include <Windows.h>
#include <winhttp.h>
#include <memory>
#include <iostream>
#include <vector>
#include "httpRequest.h"
#pragma comment(lib,"winhttp.lib")

bool request()
{
	//HTTPセッションの詳細を保持するハンドル
	std::shared_ptr<std::remove_pointer<HINTERNET>::type> hSession(
		WinHttpOpen(L"WinHTTP Example/1.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0),
		//			UA						IEプロキシ設定を踏襲				 固定					固定					 固定	
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

	WinHttpSetCredentials(hRequest.get(), WINHTTP_AUTH_TARGET_SERVER, WINHTTP_AUTH_SCHEME_BASIC, L"user", L"pass", nullptr);

	bRet = WinHttpSendRequest(hRequest.get(), 
		WINHTTP_NO_ADDITIONAL_HEADERS,	//追加のヘッダ
		0,								//追加のヘッダサイズ
		WINHTTP_NO_REQUEST_DATA,		//POSTメソッド等で送るボディ
		0,								//第4引数のサイズ
		0,								//content-lengthヘッダの指定
		0								//コールバックに渡す引数
	);
	if (!bRet)
	{
		std::cerr << "WinHttpSendRequest failed with error: " << GetLastError() << std::endl;
		return false;
	}

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
	return true;
}