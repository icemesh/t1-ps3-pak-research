/******************************************************************************/
/*
  Author  - icemesh
*/
/******************************************************************************/
#include <stdio.h>

#include "package/package.h"
#include "utils/utils.h"

int main(int argc, const char* aArgv[])
{
	puts("-------------[ t1-ps3 pak loader by icemesh ]--------------\n");
	if (argc > 1)
	{
#ifdef WIN32
		static_assert(sizeof(char*) == 0x4, "The ps3 has 4bytes pointers. Please use the x86 build");
#endif
		Package* pPackage = new Package(aArgv[1]);
		if (pPackage)
		{
			if (pPackage->PackageLogin())
			{
				MsgWarn("Further data processing goes here");
			}
			delete pPackage;
		}
		else
		{
			MsgErr("Failed to allocate package class\n");
		}
	}
	else
	{
		puts("Usage: main.exe <name>.pak (PS3 only)");
	}
}