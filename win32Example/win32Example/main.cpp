#include <tchar.h>
#include "getaddrinfo.h"
int _tmain(int argc, _TCHAR* argv[])
{
	int ret = ResolveAddr("www.microsoft.com");
	return ret;
}