#include "wsc.h"
#include <iostream>
#include <Windows.h>
#include <Wscapi.h>
#include <string>
#pragma comment(lib, "Wscapi.lib")

static std::string printStatus(WSC_SECURITY_PROVIDER_HEALTH status);

void getHealth()
{
    WSC_SECURITY_PROVIDER_HEALTH health;
    if (S_OK == WscGetSecurityProviderHealth(WSC_SECURITY_PROVIDER_FIREWALL, &health))
    {
        std::cout << "FIREWALL:\t" << printStatus(health) << std::endl;
    }
    if (S_OK == WscGetSecurityProviderHealth(WSC_SECURITY_PROVIDER_AUTOUPDATE_SETTINGS, &health))
    {
        std::cout << "AUTOUPDATE:\t" << printStatus(health) << std::endl;
    }
    if (S_OK == WscGetSecurityProviderHealth(WSC_SECURITY_PROVIDER_ANTIVIRUS, &health))
    {
        std::cout << "ANTIVIRUS:\t" << printStatus(health) << std::endl;
    }
    if (S_OK == WscGetSecurityProviderHealth(WSC_SECURITY_PROVIDER_ANTISPYWARE, &health))
    {
        std::cout << "ANTISPYWARE:\t" << printStatus(health) << std::endl;
    }
    if (S_OK == WscGetSecurityProviderHealth(WSC_SECURITY_PROVIDER_INTERNET_SETTINGS, &health))
    {
        std::cout << "INTERNET SETTINGS:\t" << printStatus(health) << std::endl;
    }
    if (S_OK == WscGetSecurityProviderHealth(WSC_SECURITY_PROVIDER_USER_ACCOUNT_CONTROL, &health))
    {
        std::cout << "UAC:\t" << printStatus(health) << std::endl;
    }
    if (S_OK == WscGetSecurityProviderHealth(WSC_SECURITY_PROVIDER_SERVICE, &health))
    {
        std::cout << "SERVICE:\t" << printStatus(health) << std::endl;
    }
    if (S_OK == WscGetSecurityProviderHealth(WSC_SECURITY_PROVIDER_ALL, &health))
    {
        std::cout << "ALL:\t" << printStatus(health) << std::endl;
    }
}

std::string printStatus(WSC_SECURITY_PROVIDER_HEALTH status)
{
    switch (status)
    {
    case WSC_SECURITY_PROVIDER_HEALTH_GOOD: return "GOOD";
    case WSC_SECURITY_PROVIDER_HEALTH_NOTMONITORED: return "NOTMONITORED";
    case WSC_SECURITY_PROVIDER_HEALTH_POOR: return "POOR";
    case WSC_SECURITY_PROVIDER_HEALTH_SNOOZE: return "SNOOZE";
    default: return "Status Error";
    }
}