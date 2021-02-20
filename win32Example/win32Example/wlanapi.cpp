#include <Windows.h>
#include <wlanapi.h>
#include <memory>
#include <iostream>
#pragma comment(lib, "wlanapi.lib")

bool enumWlanInterface()
{
	HANDLE hClient;
	DWORD dwVer;

	//第二引数はnullptrにする
	DWORD dwRet = WlanOpenHandle(WLAN_API_MAKE_VERSION(2, 0), nullptr, &dwVer, &hClient);
	if (dwRet != ERROR_SUCCESS)
	{
		std::wcerr << "WlanOpenHandle failed with error: " << dwRet << std::endl;
		return false;
	}

	std::shared_ptr<void> pWlanClient(
		hClient,
		[](HANDLE handle)
		{
			WlanCloseHandle(handle, nullptr);
		}
	);

	WLAN_INTERFACE_INFO_LIST* wlanList;
	//Wlanインターフェースを取得する
	WlanEnumInterfaces(pWlanClient.get(), nullptr, &wlanList);
	std::shared_ptr<WLAN_INTERFACE_INFO_LIST> pWlanList(wlanList, WlanFreeMemory);

	for (int i = 0; i < pWlanList->dwNumberOfItems; i++)
	{
		std::wcerr << pWlanList->InterfaceInfo[i].strInterfaceDescription << std::endl;
	}

	return true;
}