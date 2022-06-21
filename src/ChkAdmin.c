// Solution:            ChkAdmin
// Project:              ChkAdmin_C
// FileName:          ChkAdmin.c
// 
// Written by:       Doron Barzilay (DoronB)
// Creation date:  2022-06-20 12:13
// Last update:     2022-06-21 17:40

#include <malloc.h>
#include <stdio.h>
#include <Windows.h>

#pragma comment (lib,"Advapi32.lib")

int main(int argc, char** argv)
{
	BOOL quiet = FALSE;
	DWORD cbSid = SECURITY_MAX_SID_SIZE;
	// PSID pSid = _alloca(cbSid);
	PSID pSid = _malloca(cbSid);
	BOOL isAdmin = 0;

	if (argc > 1)
	{
		if (!strcmp(argv[1], "/q") || !strcmp(argv[1], "-q") || !strcmp(argv[1], "/Q") || !strcmp(argv[1], "-Q"))
		{
			quiet = TRUE;
		}
		else
		{
			if (!strcmp(argv[1], "/?"))
			{
				fprintf(stderr, "Usage: %s [/q]\n", argv[0]);
				return 0;
			}
		}
	}

	if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, pSid, &cbSid))
	{
		fprintf(stderr, "CreateWellKnownSid: error %lu\n", GetLastError());
		exit(-1);
	}

	//if (pSid != 0)
	//{
	if (!CheckTokenMembership(NULL, pSid, &isAdmin))
	{
		fprintf(stderr, "CheckTokenMembership: error %lu\n", GetLastError());
		exit(-1);
	}
	//}

	if (!quiet)
	{
		puts(isAdmin ? "Admin" : "Non-admin");
	}
	return !isAdmin;
}
