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

	//サーバーの設定
	std::shared_ptr<std::remove_pointer<HINTERNET>::type> hConnect(
		WinHttpConnect(hSession.get(),
			L"www.microsoft.com",		//FQDN or IPアドレス文字列
			INTERNET_DEFAULT_HTTPS_PORT,//ポート番号。このフラグの場合は後でWINHTTP_FLAG_SECUREを設定する(WinHttpOpenRequest)
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
			nullptr,						//要求するリソース(ファイル名やモジュール名等)
			nullptr,						//HTTPバージョン。nullptr ---> HTTP1.1
			WINHTTP_NO_REFERER,				//参照元URLの指定。WINHTTP_NO_REFERER ---> 参照元ドキュメントの未指定
			WINHTTP_DEFAULT_ACCEPT_TYPES,	//accept-typesヘッダの指定
			WINHTTP_FLAG_SECURE				//HTTPSの場合はこのフラグを指定
		),
		WinHttpCloseHandle
	);
	if (hRequest == nullptr)
	{
		std::cerr << "WinHttpOpenRequest failed with error: " << GetLastError() << std::endl;
		return false;
	}

	bool bRet = WinHttpSendRequest(hRequest.get(), 
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

	DWORD dwSize;
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
			std::cerr << szBuffer.data();
		}

	} while (dwSize > 0);
	return true;
}