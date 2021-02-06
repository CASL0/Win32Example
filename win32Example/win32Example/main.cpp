#include <tchar.h>
#include "securityDescriptor.h"
int _tmain(int argc, _TCHAR* argv[])
{
	int ret = SetAce();
	return ret;
}