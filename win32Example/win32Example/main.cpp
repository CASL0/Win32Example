#include <tchar.h>
#include <iostream>
#include "installProvider.h"
int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "japanese");

	// 5fb216a8-e2e8-4024-b853-391a4168641e
	const GUID PROVIDER_KEY =
	{
	   0x5fb216a8,
	   0xe2e8,
	   0x4024,
	   { 0xb8, 0x53, 0x39, 0x1a, 0x41, 0x68, 0x64, 0x1e }
	};

	// 0CE96AA8-0036-4CE7-B281-AC429D476995
	const GUID SUBLAYER_KEY =
	{
		0x0CE96AA8,
		0x0036,
		0x4CE7,
		{ 0xB2 ,0x81 ,0xAC ,0x42 ,0x9D ,0x47 ,0x69 ,0x95 }
	};

	InstallProvider(&PROVIDER_KEY, L"Win32Example", &SUBLAYER_KEY, L"ExampleSublayer");
	return 0;
}
