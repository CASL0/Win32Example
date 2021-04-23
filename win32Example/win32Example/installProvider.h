#pragma once
#include <Windows.h>
#include <string>

DWORD InstallProvider(__in const GUID* providerKey, __in const std::wstring& providerName, __in const GUID* subLayerKey, __in const std::wstring& subLayerName);