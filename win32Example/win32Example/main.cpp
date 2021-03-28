#include <tchar.h>
#include <iostream>
#include "adapterInfo.h"
int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "japanese");
	getAdapterInfo();
	return 0;
}
