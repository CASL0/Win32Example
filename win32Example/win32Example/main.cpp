#include <tchar.h>
#include "service.h"
int _tmain(int argc, _TCHAR* argv[])
{
	GetServiceSD(_T("wuauserv"));
	return 0;
}