#include <Windows.h>
#include <iostream>
#include "bluetoothapi.h"
#include <bluetoothapis.h>
#pragma comment(lib,"Bthprops.lib")

bool enumBthDevices()
{
	BLUETOOTH_DEVICE_SEARCH_PARAMS bthParams = { 0 };
	bthParams.dwSize = sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS);

	//�F�ؕt���f�o�C�X���T�[�`���邩�ǂ���
	bthParams.fReturnAuthenticated = true;

	//�L���ς݃f�o�C�X���T�[�`���邩�ǂ���
	bthParams.fReturnRemembered = true;

	//�y�A�����O�������ƂȂ��f�o�C�X���T�[�`���邩�ǂ���
	bthParams.fReturnUnknown = true;

	//�ڑ��ς݃f�o�C�X���T�[�`���邩�ǂ���
	bthParams.fReturnConnected = true;

	//�V�K�N�G���𔭍s
	bthParams.fIssueInquiry = true;

	//�^�C���A�E�g���w��
	//  * �^�C���A�E�g = 1.28 x cTimeoutMultiplier
	//  * �ő��48
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