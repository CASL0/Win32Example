#include "adapterInfo.h"
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <memory>
#include <iostream>
#include <vector>
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")


bool getAdapterInfo()
{
	ULONG ulFlags = GAA_FLAG_INCLUDE_PREFIX;
	ULONG ulBufLen;
	ULONG ulRet = GetAdaptersAddresses(AF_UNSPEC, ulFlags, nullptr, nullptr, &ulBufLen);
	std::shared_ptr<std::remove_pointer< PIP_ADAPTER_ADDRESSES>::type> pAddresses(
		(PIP_ADAPTER_ADDRESSES)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ulBufLen),
		[](PIP_ADAPTER_ADDRESSES pMem)
		{
			HeapFree(GetProcessHeap(), 0, pMem);
		}
	);

	ulRet = GetAdaptersAddresses(AF_UNSPEC, ulFlags, nullptr, pAddresses.get(), &ulBufLen);
	if (ulRet != ERROR_SUCCESS)
	{
		return false;
	}
	
	PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses.get();
	for (; pCurrAddresses; pCurrAddresses = pCurrAddresses->Next)
	{
		if (pCurrAddresses->OperStatus != IfOperStatusUp)
		{
			continue;
		}

		if (pCurrAddresses->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
		{
			continue;
		}

		std::wcout << "Friendly name: " << pCurrAddresses->FriendlyName << std::endl;
		PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddresses->FirstUnicastAddress;
		while (pUnicast)
		{
			SOCKADDR *pAddr= pUnicast->Address.lpSockaddr;
			ADDRESS_FAMILY af = pAddr->sa_family;

			std::vector<CHAR> szAddr(NI_MAXHOST);
			switch (af)
			{
			case AF_INET:
			{
				inet_ntop(af, &((SOCKADDR_IN*)pAddr)->sin_addr, szAddr.data(), szAddr.size());
				break;
			}
			case AF_INET6:
			{
				inet_ntop(af, &((SOCKADDR_IN6*)pAddr)->sin6_addr, szAddr.data(), szAddr.size());
				break;
			}
			default:
				break;
			}

			std::cout << "\tIP Address: " << szAddr.data() << std::endl;

			pUnicast = pUnicast->Next;
		}

		IP_ADAPTER_DNS_SERVER_ADDRESS* pDnsServer = pCurrAddresses->FirstDnsServerAddress;
		while (pDnsServer)
		{
			size_t addrLength = pDnsServer->Address.iSockaddrLength;
			std::vector<CHAR> szAddr(256);
			switch (addrLength)
			{
			case sizeof(SOCKADDR_IN):
			{
				inet_ntop(AF_INET, &((SOCKADDR_IN*)pDnsServer->Address.lpSockaddr)->sin_addr, szAddr.data(), szAddr.size());
				break;
			}
			case sizeof(SOCKADDR_IN6):
			{
				inet_ntop(AF_INET6, &((SOCKADDR_IN6*)pDnsServer->Address.lpSockaddr)->sin6_addr, szAddr.data(), szAddr.size());
				break;
			}
			default:
				break;
			}
			std::cout << "\tDNS Server: " << szAddr.data() << std::endl;
			pDnsServer = pDnsServer->Next;
		}

	}
	return true;
}