#include <WinSock2.h>
#include <iphlpapi.h>
#include <Windows.h>
#include <memory>
#include <iostream>

#pragma comment(lib, "IPHLPAPI.lib")

bool GetNetInfo(void)
{
	ULONG ulOutBufLen = sizeof(FIXED_INFO);
	DWORD dwRet;
	dwRet = GetNetworkParams(nullptr, &ulOutBufLen);
	std::shared_ptr<FIXED_INFO> pFixedInfo((FIXED_INFO*)malloc(ulOutBufLen));

	dwRet = GetNetworkParams(pFixedInfo.get(), &ulOutBufLen);
	if (dwRet == NO_ERROR)
	{
		std::cout << "Host Name: " << pFixedInfo->HostName << std::endl;
		std::cout << "Domain Name: " << pFixedInfo->DomainName << std::endl;
		std::cout << "Dns Server: " << pFixedInfo->DnsServerList.IpAddress.String << std::endl;
		return true;
	}
	return false;
}