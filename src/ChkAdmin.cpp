// Solution:            ChkAdmin
// Project:              ChkAdmin_CPP
// FileName:          ChkAdmin.cpp
// 
// Written by:       Doron Barzilay (DoronB)
// Creation date:  2022-06-20 12:13
// Last update:     2022-06-21 17:40

#include <malloc.h>
#include <cstdio>
#include <Windows.h>

#include <cstring>
#include <cctype>
#include <string>

#include "atlstr.h"
#include <regex>
#include <conio.h>

#pragma comment (lib,"Advapi32.lib")

int main(int argc, char* argv[])
{
	bool quiet = false;
	bool pause = false;
	unsigned long cbSid = SECURITY_MAX_SID_SIZE;
	// PSID pSid = _alloca(cbSid);
	void* pSid = _malloca(cbSid);
	BOOL isAdmin = FALSE;

	if (argc > 1)
	{
		if (std::regex_match(std::string(CT2CA(CString(argv[1]))),
		                     std::regex(R"(^(((--)|[\-\/]?)(\?|h(elp)?))$)", std::regex_constants::icase)))
		{
			fprintf(stderr,
#pragma region Usage (command... ..
			        R"(Usage (command line argument can start with either "--", "-" or "/"):

drive:\path\to\%s [-|--|/]<options>

-h(elp)	- show this help
-q(uiet)	- no output or keyboard waiting
-p(ause)	- wait for a keypress on end
)", argv[0]);
#pragma endregion .. ...on end

			return 0;
		}

		for (int i = 1; i < argc; i++)
		{
			if (!quiet)
			{
				quiet = std::regex_match(std::string(CT2CA(CString(argv[i]))),
				                         std::regex(R"(([\-\/])?(q)(uiet)?)", std::regex_constants::icase));
				break;
			}

			if (!pause)
			{
				pause = std::regex_match(std::string(CT2CA(CString(argv[i]))),
				                         std::regex(R"(([\-\/])?(p)(ause)?)", std::regex_constants::icase));
			}
		}
	}

	if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, nullptr, pSid, &cbSid))
	{
		if (!quiet)
		{
			fprintf(stderr, "CreateWellKnownSid: error %lu\n", GetLastError());
		}

		exit(-1);
	}

	if (pSid != nullptr)
	{
		if (!CheckTokenMembership(nullptr, pSid, &isAdmin))
		{
			if (!quiet)
			{
				fprintf(stderr, "CheckTokenMembership: error %lu\n", GetLastError());
			}

			exit(-1);
		}
	}

	if (!quiet)
	{
		puts(isAdmin ? "Admin" : "Non-admin");

		if (pause)
		{
			auto ch = _getch();
		}
	}

	return !isAdmin;
}

#pragma region Modified to use regex
////auto csArg = CString(argv[1]).MakeLower();
////CT2CA pszConvertedAnsiString(csArg);
////std::string strStd(pszConvertedAnsiString);
////const std::regex pattern(/*"([\\-/])([Qq])([IiCcKk])"*/R"(([\-\/])?(q)(uick)?)", std::regex_constants::icase);

//if (std::regex_match(std::string(CT2CA(CString(argv[1]))),
//                     std::regex(R"(([\-\/])?(q)(uiet)?)", std::regex_constants::icase)))
//{
//	quiet = true;
//}
//else
//{
//	if (!strcmp(argv[1], "/?"))
//	{
//		fprintf(stderr, "Usage: %s [/q]\n", argv[0]);
//		return 0;
//	}
//}
#pragma endregion Modified to use regex

#pragma region Original source
//#include <malloc.h>
//#include <stdio.h>
//#include <windows.h>
//#pragma comment (lib,"Advapi32.lib")
//
//int main(int argc, char** argv) {
//	BOOL quiet = FALSE;
//	DWORD cbSid = SECURITY_MAX_SID_SIZE;
//	PSID pSid = _alloca(cbSid);
//	BOOL isAdmin;
//
//	if (argc > 1) {
//		if (!strcmp(argv[1], "/q")) quiet = TRUE;
//		else if (!strcmp(argv[1], "/?")) { fprintf(stderr, "Usage: %s [/q]\n", argv[0]); return 0; }
//	}
//
//	if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, pSid, &cbSid)) {
//		fprintf(stderr, "CreateWellKnownSid: error %d\n", GetLastError()); exit(-1);
//	}
//
//	if (!CheckTokenMembership(NULL, pSid, &isAdmin)) {
//		fprintf(stderr, "CheckTokenMembership: error %d\n", GetLastError()); exit(-1);
//	}
//
//	if (!quiet) puts(isAdmin ? "Admin" : "Non-admin");
//	return !isAdmin;
//}
#pragma endregion Original source
