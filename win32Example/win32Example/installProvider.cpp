#include "installProvider.h"
#include <iostream>
#include <fwpmu.h>

#pragma comment(lib, "fwpuclnt.lib")

DWORD InstallProvider(
    __in const GUID* providerKey,
    __in const std::wstring& providerName,
    __in const GUID* subLayerKey,
    __in const std::wstring& subLayerName)
{
    const std::wstring SESSION_NAME = L"Win32Example";
    FWPM_SESSION0 session;
    ZeroMemory(&session, sizeof(session));
    session.displayData.name = const_cast<WCHAR*>(SESSION_NAME.c_str());
    session.txnWaitTimeoutInMSec = INFINITE;

    HANDLE engine = nullptr;
    DWORD dwRet = FwpmEngineOpen0(nullptr, RPC_C_AUTHN_DEFAULT, nullptr, &session, &engine);
    if (dwRet != ERROR_SUCCESS)
    {
        std::cout << "FwpmEngineOpen0 failed with error: " << dwRet << std::endl;
        goto CLEANUP;
    }

    //ロールバックできるようにする
    //commitしてない場合、enginecloseした場合破棄される
    dwRet = FwpmTransactionBegin0(engine, 0);
    if (dwRet != ERROR_SUCCESS)
    {
        std::cout << "FwpmTransactionBegin0 failed with error: " << dwRet << std::endl;
        goto CLEANUP;
    }

    FWPM_PROVIDER0 provider;
    ZeroMemory(&provider, sizeof(provider));
    provider.providerKey = *providerKey;
    provider.displayData.name = const_cast<WCHAR*>(providerName.c_str());
    provider.flags = FWPM_PROVIDER_FLAG_PERSISTENT;

    dwRet = FwpmProviderAdd0(engine, &provider, nullptr);
    if (dwRet != FWP_E_ALREADY_EXISTS && dwRet != ERROR_SUCCESS)
    {
        std::cout << "FwpmProviderAdd0 failed with error: " << dwRet << std::endl;
        goto CLEANUP;
    }

    FWPM_SUBLAYER0 subLayer;
    ZeroMemory(&subLayer, sizeof(subLayer));
    subLayer.subLayerKey = *subLayerKey;
    subLayer.displayData.name = const_cast<WCHAR*>(subLayerName.c_str());
    subLayer.flags = FWPM_SUBLAYER_FLAG_PERSISTENT;
    subLayer.providerKey = (GUID*)providerKey;
    subLayer.weight = 0xFFFE;

    dwRet = FwpmSubLayerAdd0(engine, &subLayer, nullptr);
    if (dwRet != FWP_E_ALREADY_EXISTS && dwRet != ERROR_SUCCESS)
    {
        std::cout << "FwpmSubLayerAdd0 failed with error: " << dwRet << std::endl;
        goto CLEANUP;
    }

    dwRet = FwpmTransactionCommit0(engine);
    if (dwRet != ERROR_SUCCESS)
    {
        std::cout << "FwpmTransactionCommit0 failed with error: " << dwRet << std::endl;
        goto CLEANUP;
    }


CLEANUP:
    FwpmEngineClose0(engine);
    return dwRet;
}

DWORD Uninstall(__in const GUID* providerKey, __in const GUID* subLayerKey)
{
    const std::wstring SESSION_NAME = L"Win32Example";
    FWPM_SESSION0 session;
    ZeroMemory(&session, sizeof(session));
    session.displayData.name = const_cast<WCHAR*>(SESSION_NAME.c_str());
    session.txnWaitTimeoutInMSec = INFINITE;

    HANDLE engine = nullptr;
    DWORD dwRet = FwpmEngineOpen0(nullptr, RPC_C_AUTHN_DEFAULT, nullptr, &session, &engine);
    if (dwRet != ERROR_SUCCESS)
    {
        std::cout << "FwpmEngineOpen0 failed with error: " << dwRet << std::endl;
        goto CLEANUP;
    }

    //ロールバックできるようにする
    //commitしてない場合、enginecloseした場合破棄される
    dwRet = FwpmTransactionBegin0(engine, 0);
    if (dwRet != ERROR_SUCCESS)
    {
        std::cout << "FwpmTransactionBegin0 failed with error: " << dwRet << std::endl;
        goto CLEANUP;
    }
    
    dwRet = FwpmSubLayerDeleteByKey0(engine, subLayerKey);
    if (dwRet != FWP_E_SUBLAYER_NOT_FOUND && dwRet != ERROR_SUCCESS)
    {
        std::cout << "FwpmSubLayerDeleteByKey0 failed with error: " << dwRet << std::endl;
        goto CLEANUP;
    }

    dwRet = FwpmProviderDeleteByKey0(engine, providerKey);
    if (dwRet != FWP_E_PROVIDER_NOT_FOUND && dwRet != ERROR_SUCCESS)
    {
        std::cout << "FwpmProviderDeleteByKey0 failed with error: " << dwRet << std::endl;
        goto CLEANUP;
    }

    dwRet = FwpmTransactionCommit0(engine);
    if (dwRet != ERROR_SUCCESS)
    {
        std::cout << "FwpmTransactionCommit0 failed with error: " << dwRet << std::endl;
        goto CLEANUP;
    }


CLEANUP:
    FwpmEngineClose0(engine);
    return dwRet;
}