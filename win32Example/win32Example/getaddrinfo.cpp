#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <iostream>
#include <vector>

#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable : 4996) //inet_ntoa, inet_ntop用

int ResolveAddr(const std::string& sFqdn)
{
	int iRet;
	WSAData wsaData;
    iRet= WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iRet != 0) {
        std::cout << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    /*
    * ai_familyの値がAF_UNSPECの場合、v4とv6を受け入れる。(AF_UNSPECとPF_UNSPECは同じ)
    * ai_socktypeの値が 0 の場合、呼び出し元が任意のソケット型を受け入れる。
    * ai_protocolの値が 0 の場合、呼び出し元が任意のプロトコルを受け入れる。
    * ai_addrlenメンバは 0 に設定する必要があります。
    * ai_canonnameメンバはNULLに設定する必要があります。
    * ai_addrメンバはNULLに設定する必要があります。
    * ai_nextメンバはNULLに設定する必要があります。
    */
    ADDRINFO hints = { 0 };
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_ALL;

    DWORD dwRet;
    ADDRINFO* pResult = nullptr;
    dwRet = getaddrinfo(sFqdn.c_str(), nullptr, &hints, &pResult);
    if (dwRet != 0) 
    {
        std::cout << "getaddrinfo failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    SOCKADDR_IN* sockaddrIpv4 = nullptr;
    SOCKADDR_IN6* sockaddrIpv6 = nullptr;
    int i = 1;
    for (ADDRINFO* ptr = pResult; ptr != nullptr; ptr = ptr->ai_next)
    {
        std::cout << "getaddrinfo response [" << i++ << "]" << std::endl;
        std::cout << "\tFlags: 0x" << std::hex << ptr->ai_flags << std::endl;
        std::cout << "\tFamily: ";
        switch (ptr->ai_family)
        {
        case AF_UNSPEC:
            std::cout << "Unspecified" << std::endl;
            break;
        case AF_INET:
            std::cout << "AF_INET (IPv4)" << std::endl;
            sockaddrIpv4 = (SOCKADDR_IN*)ptr->ai_addr;
            std::cout << "\tIPv4 address " << inet_ntoa(sockaddrIpv4->sin_addr) << std::endl;
            break;
        case AF_INET6:
        {
            std::cout << "AF_INET6 (IPv6)" << std::endl;
            sockaddrIpv6 = (SOCKADDR_IN6*)ptr->ai_addr;
            std::vector<CHAR> ipBuffer(50);
            std::cout << "\tIPv6 address " << inet_ntop(AF_INET6, &sockaddrIpv6->sin6_addr, ipBuffer.data(), 50) << std::endl;
        }
            break;
        case AF_NETBIOS:
            std::cout << "AF_NETBIOS (NetBIOS)" << std::endl;
            break;
        default:
            std::cout << "Other " << ptr->ai_family << std::endl;
            break;
        }
        std::cout << "\tSocket type: ";
        switch (ptr->ai_socktype)
        {
        case 0:
            std::cout << "Unspecified" << std::endl;
            break;
        case SOCK_STREAM:
            std::cout << "SOCK_STREAM (stream)" << std::endl;
            break;
        case SOCK_DGRAM:
            std::cout << "SOCK_DGRAM (datagram)" << std::endl;
            break;
        case SOCK_RAW:
            std::cout << "SOCK_RAW (raw)" << std::endl;
            break;
        case SOCK_RDM:
            std::cout << "SOCK_RDM (reliable message datagram" << std::endl;
            break;
        case SOCK_SEQPACKET:
            std::cout << "SOCK_SEQPACKET (pseudo-stream packet)" << std::endl;
            break;
        default:
            std::cout << "Other " << ptr->ai_socktype << std::endl;
            break;
        }

        std::cout << "\tProtocol: ";
        switch (ptr->ai_protocol)
        {
        case 0:
            std::cout << "Unspecified" << std::endl;
            break;
        case IPPROTO_TCP:
            std::cout << "IPPROTO_TCP (TCP)" << std::endl;
            break;
        case IPPROTO_UDP:
            std::cout << "IPPROTO_UDP (UDP)" << std::endl;
            break;
        default:
            std::cout << "Other " << ptr->ai_protocol << std::endl;
            break;
        }
        std::cout << "\tLength of this sockaddr: " << ptr->ai_addrlen << std::endl;
    }

    freeaddrinfo(pResult);
    WSACleanup();
    return 0;
}