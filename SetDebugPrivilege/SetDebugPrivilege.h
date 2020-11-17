#pragma once

#include <Windows.h>
#include <vector>
#include <tchar.h>
#include <TlHelp32.h>

struct PROCESS_DATA
{
	TCHAR szExeFileName[MAX_PATH];
	DWORD PID;
};

void ListProcesses(std::vector<PROCESS_DATA> & List);
bool SetPrivilege(DWORD PID);