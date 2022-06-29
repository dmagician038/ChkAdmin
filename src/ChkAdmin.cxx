// Solution:            ChkAdmin
// Project:              ChkAdmin_CXX
// FileName:          ChkAdmin.cxx
// 
// Written by:       Doron Barzilay (DoronB)
// Creation date:  2022-06-20 12:13
// Last update:     2022-06-21 17:41

#pragma region Includes

#include <malloc.h>
#include <stdio.h>
#include <conio.h>
#include <Windows.h>

#pragma endregion Includes

#pragma comment (lib,"Advapi32.lib")

#define MAXIMUM_STRING_LENGTH 					(const unsigned int)512

#define DEFAULT_QUIET 							(const BOOL)FALSE
#define DEFAULT_PAUSE 							(const BOOL)FALSE

#define DEFAULT_TIMEOUT_IN_MILLISECONDS 		(const unsigned int)10000	// 10 Seconds
#define MAXIMUM_TIMEOUT_IN_MILLISECONDS 		(const unsigned int)600000	// 10 Minutes
#define THREAD_SLEEP_DELAY_IN_MILLISECONDS 		(const unsigned int)100		// One Hundredth (0.100) of a Second

#pragma region Help text format

#define HELP_TEXT   (const char* const)"syntax:	\"%s\" [-|--|/]<option>\r\n\
\r\n\
	/? | -h | /h | --help    | /help    - Help\r\n\
	     -v | /v | --verbose | /verbose - Verbose output and pause (depnds on 'p')\r\n\
	     -p | /p | --pause   | /pause   - Wait for a keypress on end\r\n\
\r\n\
Exit codes:\r\n\
	 0	- ADMIN\r\n\
	 1	- Non-ADMIN\r\n\
	-1	- Help shown\r\n\
	-2	- Invalid argument given\r\n\
	-3	- Unknown SID\r\n\
	-4	- User can't check membership\r\n"
static const char* const H_ELP_TEXT = /* "\r\n"*/
//"Usage (command line argument can start with either '--', '-' or '/')"
//"\r\n"
//"\r\n"
"syntax:	\"%s\" [-|--|/]<option>\r\n"
"\r\n"
"	/? | -h | /h | --help    | /help    - Help\r\n"
"	     -v | /v | --verbose | /verbose - Verbose output and pause (depnds on 'p')\r\n"
"	     -p | /p | --pause   | /pause   - Wait for a keypress on end\r\n"
"\r\n"
"Exit codes:\r\n"
"	 0	- ADMIN\r\n"
"	 1	- Non-ADMIN\r\n"
"	-1	- Help shown\r\n"
"	-2	- Invalid argument given\r\n"
"	-3	- Unknown SID\r\n"
"	-4	- User can't check membership\r\n";

#pragma endregion Help text format

#pragma region Remove Leading / Trailing blanks (SPACE, TAB, CR, LF)

char* trim(char* string)
{
	char* ptr = NULL;

	while (*string == ' ' || *string == '\t' || *string == '\r' || *string == '\n')
	{
		string++;
	} // while (*string == ' ' || *string == '\t' || *string == '\r' || *string == '\n') …

	ptr = string + strlen(string) - 1; // jump to the last char (-1 because '\0')

	while (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n')
	{
		*ptr = '\0';
		ptr--;
	} // while (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\n') …

	return string; // return pointer to the modified start 
}

#pragma endregion  Remove Leading / Trailing blanks (SPACE, TAB, CR, LF)

#pragma region Wait for user input (or predefined timeout)

void pause(int exitCode, const unsigned int iDelayInMilliSeconds, BOOL bQuiet)
{
	BOOL bPause = iDelayInMilliSeconds != 0 && iDelayInMilliSeconds <= MAXIMUM_TIMEOUT_IN_MILLISECONDS;

	if (!bQuiet/* && bPause*/)
	{
		// Waiting for 0 seconds, press a key to continue
		//char const* const FinishMessageFormat 	= "Exit code: %d (%s)\r\nWaiting for [s%i[u seconds)[K seconds, press a key to continue ...";
		auto const FinishMessageFormat = "Exit code: %d (%s)\r\n";
		auto const TimeoutMessageFormat = "\r\nWaiting for [s%3i seconds, press a key to continue ...[K[u";// "\r\nWaiting for [s%i[u seconds, press a key to continue ...[K";
		auto const CountDownFormat = "[s%3i[u";

		char exitMessage[MAXIMUM_STRING_LENGTH] = { 0 };// */ = "Invalid arguments:\t";// = "\r\nInvalid arguments:\t\0";
		char msg2[MAXIMUM_STRING_LENGTH] = { 0 };// */ = "Invalid arguments:\t";// = "\r\nInvalid arguments:\t\0";

		errno_t e = _strset_s(exitMessage, MAXIMUM_STRING_LENGTH, 0);
		strcat_s(exitMessage, MAXIMUM_STRING_LENGTH, ""/*"Invalid command line arguments:\t"*/);
		e = _strset_s(msg2, MAXIMUM_STRING_LENGTH, 0);
		strcat_s(msg2, MAXIMUM_STRING_LENGTH, ""/*"Invalid command line arguments:\t"*/);

		int chRes = 0;
		BOOL doBreak = FALSE;
		int timeOut = 0;
		int iTimeoutSeconds = iDelayInMilliSeconds / 1000 - 1;

		//sprintf_s(exitMessage, MAXIMUM_STRING_LENGTH, "\r\nExit code: %d (%%s),\r\nPress any key to continue . . .\r\nAuto-closing in ", exitCode);
		//sprintf_s(exitMessage, MAXIMUM_STRING_LENGTH, "Press any key to continue . . .\r\nAuto-closing in ");

		switch (exitCode)
		{
		case 0:
		{
			////sprintf(exitMessage, "\r\nExit code: %d (%s), Press any key to continue . . .\r\nTimeout in ", exitCode, "ADMIN");
			sprintf_s(exitMessage, sizeof(exitMessage), FinishMessageFormat, exitCode, "ADMIN");
			//sprintf_s(exitMessage, MAXIMUM_STRING_LENGTH, exitMessage, "ADMIN");

			break;
		} // case 0: …
		case 1:
		{
			////sprintf(exitMessage, "\r\nExit code: %d (%s), Press any key to continue . . .\r\nTimeout in ", exitCode, "Non-ADMIN");
			sprintf_s(exitMessage, sizeof(exitMessage), FinishMessageFormat, exitCode, "Non-ADMIN");
			//sprintf_s(exitMessage, MAXIMUM_STRING_LENGTH, exitMessage, "Non-ADMIN");

			break;
		} // case 1: …
		case -1:
		{
			////sprintf(exitMessage, "\r\nExit code: %d (%s), Press any key to continue . . .\r\nTimeout in ", exitCode, "Help shown");
			sprintf_s(exitMessage, sizeof(exitMessage), FinishMessageFormat, exitCode, "Help shown");
			//sprintf_s(msg2, MAXIMUM_STRING_LENGTH, exitMessage, "Help shown");

			break;
		} // case -1: …
		case -2:
		{
			////sprintf(exitMessage, "\r\nExit code: %d (%s), Press any key to continue . . .\r\nTimeout in ", exitCode, "Invalid argument given");
			sprintf_s(exitMessage, sizeof(exitMessage), FinishMessageFormat, exitCode, "Invalid argument given");
			//sprintf_s(exitMessage, MAXIMUM_STRING_LENGTH, exitMessage, "Invalid argument given");

			break;
		} // case -2: …
		case -3:
		{
			////sprintf(exitMessage, "\r\nExit code: %d (%s), Press any key to continue . . .\r\nTimeout in ", exitCode, "Unknown SID");
			sprintf_s(exitMessage, sizeof(exitMessage), FinishMessageFormat, exitCode, "Unknown SID");
			//sprintf_s(exitMessage, MAXIMUM_STRING_LENGTH, exitMessage, "Unknown SID");

			break;
		} // case -3: …
		case -4:
		{
			////sprintf(exitMessage, "\r\nExit code: %d (%s), Press any key to continue . . .\r\nTimeout in ", exitCode, "User can't check membership");
			sprintf_s(exitMessage, sizeof(exitMessage), FinishMessageFormat, exitCode, "User can't check membership");
			//sprintf_s(exitMessage, MAXIMUM_STRING_LENGTH, exitMessage, "User can't check membership");

			break;
		} // case -4: …
		default:
		{
			////sprintf_s(exitMessage, "\r\nExit code: %d (%s), Press any key to continue . . .\r\nTimeout in ", exitCode, "Knknown");
			sprintf_s(exitMessage, sizeof(exitMessage), FinishMessageFormat, exitCode, "Unknown");
			//sprintf_s(exitMessage, MAXIMUM_STRING_LENGTH, exitMessage, "Unknown");

			break;
		}
		} // switch (exitCode) …

		// fprintf_s(stdout, "%s\r\nPress any key to continue . . .\r\nTimeout in %c[s%i sec%c[K%c[u", exitMessage, ESCAPE_CHAR, (iTimeoutSeconds + 1), ESCAPE_CHAR, ESCAPE_CHAR);
		fprintf_s(stdout, exitMessage);// , (iTimeoutSeconds + 1), ESCAPE_CHAR, ESCAPE_CHAR);

		if (bPause)
		{
			fprintf_s(stdout, TimeoutMessageFormat, (iTimeoutSeconds + 1));// , (iTimeoutSeconds + 1), ESCAPE_CHAR, ESCAPE_CHAR);
			do
			{
				// char tmpStr[MAXIMUM_STRING_LENGTH / 32];
				Sleep(THREAD_SLEEP_DELAY_IN_MILLISECONDS);
				timeOut += THREAD_SLEEP_DELAY_IN_MILLISECONDS;

				if (iDelayInMilliSeconds - timeOut <= 0)
				{
					doBreak = TRUE;
				}

				if (timeOut % 1000 == 0)
				{
					//iTimeoutSeconds = (iDelayInMilliSeconds - timeOut) / 1000;
					fprintf(stdout, CountDownFormat, iTimeoutSeconds--);
					//iTimeoutSeconds--;
				}
			} while (!_kbhit() && !doBreak); // while (!_kbhit()) …

			// fprintf(stdout, "%c[s%i sec%c[K%c[u\r\n", (char)27, 0, (char)27, (char)27);
			puts("\r\n");

			while (!chRes && !doBreak)
			{
				chRes = _getch();
			}// do …
		}

	} // if (verbose && pause) …
}

#pragma endregion Wait for user input (or predefined timeout)

#pragma region Count how many elements in the argv parameter

int string_array_count(const char* argv[])
{
	int i = 0;

	while (argv[i] != NULL)
	{
		i++;
	} // while (argv[i] != NULL) …

	return i;
}

#pragma endregion Count how many elements in the argv parameter

#pragma region Show (extended) help message on console

void help(const char* helpText, const char* invalidArgs, int retCode)
{
	char msg1[MAXIMUM_STRING_LENGTH] = { 0 };// */ = "Invalid arguments:\t";// = "\r\nInvalid arguments:\t\0";
	//char msg2[MAXIMUM_STRING_LENGTH];// */ = "Invalid arguments:\t";// = "\r\nInvalid arguments:\t\0";
	// msg1[0] = '\0';
	//msg2[0] = '\0';

	errno_t e = _strset_s(msg1, MAXIMUM_STRING_LENGTH, 0);
	strcat_s(msg1, MAXIMUM_STRING_LENGTH, ""/*"Invalid command line arguments:\t"*/);
	//e = _strset_s(msg2, MAXIMUM_STRING_LENGTH, 0);
	//strcat_s(msg2, MAXIMUM_STRING_LENGTH, ""/*"Invalid command line arguments:\t"*/);

	sprintf_s(msg1, sizeof(msg1), HELP_TEXT, helpText);	// NOLINT(clang-diagnostic-cast-qual)
	fprintf_s(stderr, "%s\r\n", msg1);

	if (_stricmp(invalidArgs, "") != 0)
	{
		fprintf_s(stderr, "Invalid command line arguments: %s\r\n\r\n", invalidArgs);
	}

	//sprintf_s(msg1, MAXIMUM_STRING_LENGTH, "Exit code is: %d (%%s)", retCode);

	// fprintf_s(stderr, "Exit code is: %d\r\n", retCode);
	// fprintf_s(stderr, "%s\r\n", msg2);
}

#pragma endregion Show (extended) help message on console

#pragma region Evaluate command line arguments and response accordingly

int exec(const char* argv[])
{
	int argc = string_array_count(argv);
	char* arg = NULL;

	char invalidArgs[MAXIMUM_STRING_LENGTH] = { 0 };// */ = "Invalid arguments:\t";// = "\r\nInvalid arguments:\t\0";
	//invalidArgs[0] = '\0';

	errno_t e = _strset_s(invalidArgs, MAXIMUM_STRING_LENGTH, 0);
	strcat_s(invalidArgs, MAXIMUM_STRING_LENGTH, ""/*"Invalid command line arguments:\t"*/);
	// invalidArgs = _strset_s("Invalid arguments:	",MAXIMUM_STRING_LENGTH, 0);

	char helpText[MAXIMUM_STRING_LENGTH] = "";

	BOOL bHelp = FALSE;
	BOOL bQuiet = DEFAULT_QUIET;
	BOOL bPause = DEFAULT_PAUSE;
	BOOL bInvalid = FALSE;

	DWORD cbSid = SECURITY_MAX_SID_SIZE;
	PSID pSid = NULL;
	BOOL isAdmin = FALSE;

	pSid = _malloca(cbSid);

#pragma region Parse argument list given in argv

	if (argc > 1)
	{
		for (int i = 1; i < argc; i++)
		{
			arg = _strdup(argv[i]);
			size_t len = strlen(arg) + 1;
			strcat_s(arg, len, "\0");
			// strcat_s(arg, sizeof(arg), "\0");
			//arg = _strlwr((char*)argv[i]);	// NOLINT(clang-diagnostic-cast-qual)
			//errno_t a = _strlwr_s(arg, strlen(arg));
			errno_t a = _strlwr_s(arg, len);

			if (arg != 0 && (_stricmp(arg, "--help") == 0 || _stricmp(arg, "/help") == 0 ||
				_stricmp(arg, "-h") == 0 || _stricmp(arg, "/h") == 0 ||
				_stricmp(arg, "-?") == 0 || _stricmp(arg, "/?") == 0))
			{
				bHelp |= TRUE;
			} // if (_stricmp(arg, "--help") == 0 || _stricmp(arg, "/help") || …
			else
			{
				if (arg != 0 && (_stricmp(arg, "--quiet") == 0 || _stricmp(arg, "/quiet") == 0 ||
					_stricmp(arg, "-q") == 0 || _stricmp(arg, "/q") == 0))
				{
					bQuiet |= TRUE;
				} // if (_stricmp(arg, "--quiet") == 0 || _stricmp(arg, "/quiet") == 0 || …
				else
				{
					if (arg != 0 && (_stricmp(arg, "--pause") == 0 || _stricmp(arg, "/pause") == 0 ||
						_stricmp(arg, "-p") == 0 || _stricmp(arg, "/p") == 0))
					{
						bPause |= TRUE;
					} // if (_stricmp(arg, "--pause") == 0 || _stricmp(arg, "/pause") == 0 || …
					else
					{
						if (arg != 0)
						{
							bInvalid |= TRUE;
							//strcat(invalidArgs, arg);
							//strcat(invalidArgs, " ");

							strcat_s(invalidArgs, MAXIMUM_STRING_LENGTH/*sizeof(invalidArgs) + sizeof(arg)*/, argv[i]/*arg*/);
							strcat_s(invalidArgs, MAXIMUM_STRING_LENGTH/*sizeof(invalidArgs) + sizeof(" ")*/, " ");
						}
						else
						{
							bInvalid |= TRUE;
							//strcat(invalidArgs, arg);
							//strcat(invalidArgs, " ");

							strcat_s(invalidArgs, MAXIMUM_STRING_LENGTH/*sizeof(invalidArgs) + sizeof("??")*/, "??");
							strcat_s(invalidArgs, MAXIMUM_STRING_LENGTH/*sizeof(invalidArgs) + sizeof(" ")*/, " ");
						}
					} // if (_stricmp(arg, "--pause") == 0 || _stricmp(arg, "/pause") == 0 || … else
				} // if (_stricmp(arg, "--quiet") == 0 || _stricmp(arg, "/quiet") == 0 || … else
			}
		} // for (int i = 1; i < argc; i++)

		if (_stricmp(invalidArgs, "") != 0)
		{
			trim(invalidArgs);
		}

		// strcat_s(invalidArgs, MAXIMUM_STRING_LENGTH/*sizeof(invalidArgs) + sizeof("\r\n")*/, "\r\n");
	} // if (argc > 1)

#pragma endregion Parse argument list given in argv

	if (bHelp)
	{
		help(argv[0], invalidArgs, -1);

		pause(-1, bPause ? DEFAULT_TIMEOUT_IN_MILLISECONDS : 0, bQuiet);

		//exit(-1);
		return -1;
	} // if (bHelp) …

	//if (bQuiet && bInvalid)
	//{
	//	//sprintf(helpText, HELP_TEXT, argv[0]);
	//	help(argv[0], invalidArgs, -2);
	//	//strcat(invalidArgs, helpText);
	//	//strcat_s(invalidArgs, sizeof(helpText), helpText);

	//	//fprintf_s(stderr, "%s", invalidArgs);

	//	if (bInvalid)
	//	{
	//		pause(-2, bPause ? DEFAULT_TIMEOUT_IN_MILLISECONDS : 0, bQuiet);
	//		//exit(-2);
	//		return -2;

	//	} // if (bInvalid) …

	//	//pause(-1, DEFAULT_TIMEOUT_IN_MILLISECONDS, bQuiet, bPause);
	//	//exit(-1);
	//} // if (bQuiet && bInvalid) …
	////else
	////{
	////	exit(-2);
	////} // if (bQuiet && bInvalid) … else

#pragma region Evaluate ADMIN rights

	if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, pSid, &cbSid))
	{
		if (bQuiet)
		{
			fprintf(stderr, "CreateWellKnownSid: error %lu\r\nERROR:	Unknown SID\r\n", GetLastError());
		} // if (bQuiet) …

		pause(-3, bPause ? DEFAULT_TIMEOUT_IN_MILLISECONDS : 0, bQuiet);

		exit(-3);
	} // if (!CreateWellKnownSid(WinBuiltinAdministratorsSid, NULL, pSid, &cbSid)) …

	if (pSid != 0 && !CheckTokenMembership(NULL, pSid, &isAdmin))
	{
		if (bQuiet)
		{
			fprintf(stderr, "CheckTokenMembership: error %lu\nERROR:	User can't check membership\r\n", GetLastError());
		} // if (bQuiet) …

		pause(-4, bPause ? DEFAULT_TIMEOUT_IN_MILLISECONDS : 0, bQuiet);

		//exit(-4);
		return -4;
	} // if (!CheckTokenMembership(NULL, pSid, &isAdmin)) …

	isAdmin = isAdmin == 1 ? 0 : 1;

#pragma endregion Evaluate ADMIN rights

	//if (bQuiet)
	//{
	//	puts(isAdmin ? "Non-admin" : "Admin");
	//} // if (bQuiet) …

	pause(isAdmin, bPause ? DEFAULT_TIMEOUT_IN_MILLISECONDS : 0, bQuiet);

	return isAdmin;
}

#pragma endregion Evaluate command line arguments and response accordingly

#pragma region Main program entry point

int main(int argc, const char** argv)
{
	int r = exec(argv);
	exit(r);
}

#pragma endregion Main program entry point
