// Solution:            ChkAdmin
// Project:              ChkAdmin_CXX
// FileName:          ChkAdmin.cxx
// 
// Written by:       Doron Barzilay (DoronB)
// Creation date:  2022-06-20 12:13
// Last update:     2022-06-21 17:41

#include <malloc.h>
#include <cstdio>
#include <Windows.h>

#include <cstring>
#include <cctype>
#include <string>

#include "atlstr.h"
#include <regex>

#pragma comment (lib,"Advapi32.lib")

int main(int argc, char* argv[])
{
	bool quiet = false;
	unsigned long cbSid = SECURITY_MAX_SID_SIZE;
	// PSID pSid = _alloca(cbSid);
	void* pSid = _malloca(cbSid);
	BOOL isAdmin = FALSE;

	if (argc > 1)
	{
		//auto csArg = CString(argv[1]).MakeLower();
		//CT2CA pszConvertedAnsiString(csArg);
		//std::string strStd(pszConvertedAnsiString);
		//const std::regex pattern(/*"([\\-/])([Qq])([IiCcKk])"*/R"(([\-\/])?(q)(uick)?)", std::regex_constants::icase);

		if (std::regex_match(std::string(CT2CA(CString(argv[1]))),
		                     std::regex(R"(([\-\/])?(q)(uiet)?)", std::regex_constants::icase)))
		{
			quiet = true;
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

	if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, nullptr, pSid, &cbSid))
	{
		fprintf(stderr, "CreateWellKnownSid: error %lu\n", GetLastError());
		exit(-1);
	}

	if (pSid != nullptr)
	{
		if (!CheckTokenMembership(nullptr, pSid, &isAdmin))
		{
			fprintf(stderr, "CheckTokenMembership: error %lu\n", GetLastError());
			exit(-1);
		}
	}

	if (!quiet)
	{
		puts(isAdmin ? "Admin" : "Non-admin");
	}
	return !isAdmin;
}
