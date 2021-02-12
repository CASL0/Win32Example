#include <Windows.h>
#include <iostream>
#include "bluetoothapi.h"
#include <bluetoothapis.h>
#pragma comment(lib,"Bthprops.lib")

bool enumBthDevices()
{
	BLUETOOTH_DEVICE_SEARCH_PARAMS bthParams = { 0 };
	bthParams.dwSize = sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS);

	//認証付きデバイスをサーチするかどうか
	bthParams.fReturnAuthenticated = true;

	//記憶済みデバイスをサーチするかどうか
	bthParams.fReturnRemembered = true;

	//ペアリングしたことないデバイスをサーチするかどうか
	bthParams.fReturnUnknown = true;

	//接続済みデバイスをサーチするかどうか
	bthParams.fReturnConnected = true;

	//新規クエリを発行
	bthParams.fIssueInquiry = true;

	//タイムアウトを指定
	//  * タイムアウト = 1.28 x cTimeoutMultiplier
	//  * 最大は48
	bthParams.cTimeoutMultiplier = 5;

	std::cout << "Searching Bluetooth devices...\n";

	BLUETOOTH_DEVICE_INFO bthInfo = { 0 };
	bthInfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);

	HBLUETOOTH_DEVICE_FIND hBthDeviceInfo = BluetoothFindFirstDevice(&bthParams, &bthInfo);
	if (hBthDeviceInfo == nullptr)
	{
		std::cout << "BluetoothFindFirstDevice failed with error: " << GetLastError() << std::endl;
		return false;
	}

	do
	{
		std::wcout << bthInfo.szName << std::endl;
	} while (BluetoothFindNextDevice(hBthDeviceInfo, &bthInfo));

	BluetoothFindDeviceClose(hBthDeviceInfo);

	return true;
}