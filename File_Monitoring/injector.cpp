#include <iostream>
#include <windows.h>
#include <cstring>

bool DLLInjector(DWORD, const wchar_t*);

using std::cout;

int main(void)
{
	DLLInjector(9252, L"D:\\PRATHAMESH\\WINDOWS_API_HOOKING\\File_Monitoring_DLL\\x64\\Debug\\File_Monitoring_DLL.dll");
	
	return 0;
}

bool DLLInjector(DWORD pid, const wchar_t *path)
{
	SIZE_T nLength;
	HANDLE hThread;
	HANDLE hProcess;
	LPVOID lpRemoteString = NULL;
	LPVOID lpLoadLibraryW = NULL;
	
	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if(NULL == hProcess)
	{
		cout << "Failed in getting handle\n";
		return false;
	}
	
	lpLoadLibraryW = GetProcAddress(GetModuleHandle(L"KERNEL32.DLL"), "LoadLibraryW");
	if(NULL == lpLoadLibraryW)
	{
		cout << "Failed in getting load libraray function address\n";
		CloseHandle(hProcess);
		return false;
	}
	
	nLength = wcslen(path)*sizeof(wchar_t);
	
	lpRemoteString = VirtualAllocEx(hProcess, NULL, nLength, MEM_COMMIT, PAGE_READWRITE);
	if(!lpRemoteString)
	{
		cout << "Failed in allocating memory for dll path\n";
		CloseHandle(hProcess);
		return false;
	}
	
	if(!WriteProcessMemory(hProcess, lpRemoteString, path, nLength, NULL))
	{
		cout << "Failed in writing dll path in allocated memory of process\n";
		
		VirtualFreeEx(hProcess, lpRemoteString, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}
	
	hThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)lpLoadLibraryW, lpRemoteString, NULL, NULL);
	if(!hThread)
	{
		cout << "Error in creating thread\n";
		VirtualFreeEx(hProcess, lpRemoteString, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}
	else
		WaitForSingleObject(hThread, 4000);
	
	cout << "Loaded Successfully\n";
	VirtualFreeEx(hProcess, lpRemoteString, 0, MEM_RELEASE);
	CloseHandle(hProcess);
	
	return true;	
}
