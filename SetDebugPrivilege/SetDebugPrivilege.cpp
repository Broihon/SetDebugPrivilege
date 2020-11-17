#include "SetDebugPrivilege.h"

bool Attachable(DWORD PID)
{
	HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, PID);
	if (!hProc)
		return false;

	CloseHandle(hProc);

	return true;
}

void ListProcesses(std::vector<PROCESS_DATA> & List)
{
	List.clear();
	PROCESSENTRY32	PE32{ 0 };
	PE32.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
		return;
	
	BOOL bRet = Process32First(hSnap, &PE32);
	while (bRet)
	{
		if (!Attachable(PE32.th32ProcessID) || PE32.th32ProcessID == GetCurrentProcessId())
		{
			bRet = Process32Next(hSnap, &PE32);
			continue;
		}
	
		PROCESS_DATA current{ 0 };
		memcpy(current.szExeFileName, PE32.szExeFile, _tcslen(PE32.szExeFile) * sizeof(TCHAR));
		current.PID = PE32.th32ProcessID;
		List.push_back(current);
		bRet = Process32Next(hSnap, &PE32);
	}
}

bool SetPrivilege(DWORD PID)
{
	HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, PID);
	if (!hProc)
		return false;

	HANDLE hToken = nullptr;
	if (!OpenProcessToken(hProc, TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		CloseHandle(hProc);
		return false;
	}
	
	CloseHandle(hProc);

	TOKEN_PRIVILEGES TokenPrivileges = { 0 };
	TokenPrivileges.PrivilegeCount				= 1;
	TokenPrivileges.Privileges[0].Attributes	= SE_PRIVILEGE_ENABLED;

	if (!LookupPrivilegeValue(nullptr, SE_DEBUG_NAME, &TokenPrivileges.Privileges[0].Luid))
	{
		CloseHandle(hToken);
		return false;
	}

	if (!AdjustTokenPrivileges(hToken, FALSE, &TokenPrivileges, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr))
	{
		CloseHandle(hToken);
		return false;
	}

	CloseHandle(hToken);

	return true;
}