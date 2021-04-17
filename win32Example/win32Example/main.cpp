#include <tchar.h>
#include <iostream>
#include "enumUninstalledPrograms.h"
int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "japanese");
	enumInstalledPrograms();
	return 0;
}
