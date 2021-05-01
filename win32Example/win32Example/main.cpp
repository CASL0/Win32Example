#include <tchar.h>
#include <iostream>
#include "createWebShortcut.h"
int _tmain(int argc, TCHAR* argv[])
{
	setlocale(LC_ALL, "japanese");
	tstring url(_T("url"));
	tstring path(_T("Path\\hoge.url"));
	CreateShortcut(url, path);
	return 0;
}
