#include <WinSock2.h>
#include <iphlpapi.h>
#include <Windows.h>
#include <memory>
#include <iostream>

#pragma comment(lib, "IPHLPAPI.lib")

inline void* MemAlloc(size_t size);
inline bool MemFree(void* ptr);

bool GetNetInfo(void)
{
	ULONG ulOutBufLen = sizeof(FIXED_INFO);
	DWORD dwRet;
	dwRet = GetNetworkParams(nullptr, &ulOutBufLen);
	std::shared_ptr<FIXED_INFO> pFixedInfo((FIXED_INFO*)MemAlloc(ulOutBufLen), MemFree);

	dwRet = GetNetworkParams(pFixedInfo.get(), &ulOutBufLen);
	if (dwRet != NO_ERROR)
	{
		return false;
	}

	std::cout << "Host Name: " << pFixedInfo->HostName << std::endl;
	std::cout << "Domain Name: " << pFixedInfo->DomainName << std::endl;
	std::cout << "Dns Server: " << std::endl;
	std::cout << "\t" << pFixedInfo->DnsServerList.IpAddress.String << std::endl;
	IP_ADDR_STRING* pIpAddr;
	pIpAddr = pFixedInfo->DnsServerList.Next;
	while (pIpAddr)
	{
		std::cout << "\t" << pIpAddr->IpAddress.String;
		pIpAddr = pIpAddr->Next;
	}

	std::cout << "Node Type: " << std::endl;
	switch (pFixedInfo->NodeType)
	{
	case BROADCAST_NODETYPE:
		std::cout << "\tBroadcast node" << std::endl;
		break;
	case PEER_TO_PEER_NODETYPE:
		std::cout << "\tPeer to Peer node" << std::endl;
		break;
	case MIXED_NODETYPE:
		std::cout << "\tMixed node" << std::endl;
		break;
	case HYBRID_NODETYPE:
		std::cout << "\tHybrid node" << std::endl;
		break;
	default:
		std::cout << "\tUnknown node type " << std::hex << pFixedInfo->NodeType << std::endl;
		break;
	}
	return true;
}

inline void* MemAlloc(size_t size)
{
	return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
}
inline bool MemFree(void* lpMem)
{
	return HeapFree(GetProcessHeap(), 0, lpMem);
}