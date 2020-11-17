#include "main.h"
#include <Commctrl.h>
#include <algorithm>
#include <string>

#pragma comment(lib, "Comctl32.lib")

#define SS_DESCENDING 0x01000000

HINSTANCE	g_hInstance = NULL;
LPARAM		g_SortSense = 0;
HWND		g_hListView = NULL;

HWND CreateListView(HWND hParent, int x, int y, int w, int h, DWORD ex = 0);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
bool UpdateProcessList(HWND hListView);

//int WINAPI WinMain(HINSTANCE hInstance,	HINSTANCE hPrevInstance, char * lpCmdLine, int nCmdShow)
BOOL WINAPI DllMain(HINSTANCE hDll, DWORD dwReason, void * pReserved)
{	
	if (dwReason != DLL_PROCESS_ATTACH) return TRUE;
	//UNREFERENCED_PARAMETER(hPrevInstance);
	//UNREFERENCED_PARAMETER(lpCmdLine);

	SetPrivilege(GetCurrentProcessId());

	g_hInstance = GetModuleHandle(0);

	int width	= 300;
	int height	= 300;

	HWND hWnd;
	WNDCLASSEX wc{ 0 };
	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.lpfnWndProc		= WindowProc;
	wc.hInstance		= GetModuleHandle(0);
	wc.hCursor			= LoadCursor(0, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName	= TEXT("WindowClass");

	RegisterClassEx(&wc);

	hWnd = CreateWindowEx(0, TEXT("WindowClass"), TEXT("SetDebugPrivilege"), WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX, 150, 150, width, height, 0, 0, GetModuleHandle(0), nullptr);

	ShowWindow(hWnd, 5);

	RECT wnd_rect{ 0 };
	GetClientRect(hWnd, &wnd_rect);
	width = wnd_rect.right - wnd_rect.left;
	height = wnd_rect.bottom - wnd_rect.top;

	g_hListView = CreateListView(hWnd, 0, 0, width, height - 50, LVS_EX_FULLROWSELECT);

	TCHAR szText[256]{ 0 };

	LVCOLUMN lvc{ 0 };

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVS_REPORT;
	lvc.pszText = const_cast<TCHAR*>(TEXT("Processname"));
	lvc.cx = width - 70;
	LoadString(GetModuleHandle(0), 0, szText, sizeof(szText) / sizeof(TCHAR));
	ListView_InsertColumn(g_hListView, 0, &lvc);

	ZeroMemory(&lvc, sizeof(lvc));

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM | LVS_REPORT;
	lvc.pszText = const_cast<TCHAR*>(TEXT("PID"));
	lvc.cx = 50;
	LoadString(GetModuleHandle(0), 1, szText, sizeof(szText) / sizeof(TCHAR));
	ListView_InsertColumn(g_hListView, 1, &lvc);

	UpdateProcessList(g_hListView);

	ShowWindow(g_hListView, 5);

	HWND h_B_Update		= CreateWindow(TEXT("BUTTON"), TEXT("Update"),			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10,				height - 40, width / 2 - 15, 30, hWnd, NULL, GetModuleHandle(0), nullptr);
	HWND h_B_SetPriv	= CreateWindow(TEXT("BUTTON"), TEXT("Set Privilege"),	WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, width / 2 + 5,	height - 40, width / 2 - 15, 30, hWnd, NULL, GetModuleHandle(0), nullptr);
	
	ShowWindow(h_B_Update, 5);
	ShowWindow(h_B_SetPriv, 5);

	MSG msg{ 0 };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.hwnd == h_B_Update)
		{
			if (msg.message == WM_LBUTTONUP)
			{
				g_SortSense = 0;
				UpdateProcessList(g_hListView);
			}
		}

		if (msg.hwnd == h_B_SetPriv)
		{
			if (msg.message == WM_LBUTTONUP)
			{
				int index = ListView_GetNextItem(g_hListView, -1, LVNI_SELECTED);
				while (index != -1)
				{
					TCHAR szPID[8]{ 0 };
					ListView_GetItemText(g_hListView, index, 1, szPID, 8);
					TCHAR szName[sizeof(PROCESSENTRY32::szExeFile) / sizeof(TCHAR)]{ 0 };
					ListView_GetItemText(g_hListView, index, 0, szName, sizeof(szName) / sizeof(TCHAR));

					std::basic_string<TCHAR> info_string(szName);
					info_string += TEXT(" (");
					info_string += szPID;
					info_string += TEXT(")");

					DWORD PID = 0;
					#ifdef UNICODE
						PID = (DWORD)_wtoi(szPID);
					#else
						PID = (DWORD)atoi(szPID);				
					#endif

					if (PID)
					{
						if (SetPrivilege(PID))
						{
							MessageBox(NULL, TEXT("Privilege enabled."), info_string.c_str(), MB_ICONINFORMATION);
						}
						else
						{
							MessageBox(NULL, TEXT("Enabling privilege failed."), info_string.c_str(), MB_ICONERROR);
						}
					}

					index = ListView_GetNextItem(g_hListView, index, LVNI_SELECTED);
				}
			}
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);

		if (msg.message == WM_QUIT)
			break;
	}

	return (int)msg.wParam;
}

HWND CreateListView(HWND hParent, int x, int y, int w, int h, DWORD ex) 
{
    INITCOMMONCONTROLSEX icex;
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

	RECT rcClient{ 0 };
    GetClientRect(hParent, &rcClient); 

    HWND hWnd = CreateWindow(WC_LISTVIEW, TEXT(""), WS_CHILD | LVS_REPORT, x, y, w, h, hParent, NULL, g_hInstance, nullptr);

	if (ex && hWnd)
	{
		SendMessage(hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, ex, ex);
	}

	return hWnd;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_NOTIFY:
		{
			NMHDR * nmhdr = reinterpret_cast<NMHDR*>(lParam);
			if (nmhdr->hwndFrom == g_hListView && nmhdr->code == LVN_COLUMNCLICK)
			{
				NMLISTVIEW * info = reinterpret_cast<NMLISTVIEW*>(lParam);
				if ((g_SortSense & 0x00FFFFFF) == info->iSubItem)
				{
					if (g_SortSense & SS_DESCENDING)
						g_SortSense ^= SS_DESCENDING;
					else
						g_SortSense |= SS_DESCENDING;
				}
				else
				{
					g_SortSense = info->iSubItem;
				}

				UpdateProcessList(g_hListView);
			}
		}break;

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}break;

		case WM_CLOSE:
		{
			DestroyWindow(hWnd);
		}break;
		
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

bool UpdateProcessList(HWND hListView)
{
	ListView_DeleteAllItems(hListView);

	std::vector<PROCESS_DATA> data;
	ListProcesses(data);

	if (g_SortSense & 1)
		std::sort(data.begin(), data.end(), [](const PROCESS_DATA & a, const PROCESS_DATA & b) -> bool { return a.PID > b.PID; });
	else
		std::sort(data.begin(), data.end(), [](const PROCESS_DATA & a, const PROCESS_DATA & b) -> bool { return (0 < _tcsicmp(a.szExeFileName, b.szExeFileName)); });
	
	if (g_SortSense & SS_DESCENDING)
		std::reverse(data.begin(), data.end());

	if (data.empty())
		return false;

	for (auto i : data)
	{
		LVITEM item{ 0 };
		item.mask = LVIF_TEXT;
		item.pszText = i.szExeFileName;
		ListView_InsertItem(hListView, &item);

		TCHAR szPID[8]{ 0 };
		_ultot_s<8>(i.PID, szPID, 10);
		item.pszText = szPID;
		item.iSubItem = 1;
		ListView_SetItem(hListView, &item);
	}

	return false;
}