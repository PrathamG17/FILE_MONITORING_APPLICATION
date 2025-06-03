// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
//#include "hookapi.h"
#include <fstream>
#include <iostream>
#include <winbase.h>
#include <Windows.h>
#include <atlstr.h>
#include <ctime>
#include <string>
#include "detours.h"


void HookFileCopy2();
void HookMoveFileExW();
void FileWrite(std::string, DWORD);
BOOL userCopyFile(LPCTSTR, LPCTSTR, BOOL);
BOOL userMoveFileExW(LPCWSTR, LPCWSTR, DWORD);

typedef BOOL(WINAPI* WINAPI_CopyPointer)(LPCTSTR, LPCTSTR, BOOL);
typedef BOOL(WINAPI* WINAPI_MovePointer)(LPCWSTR, LPCWSTR, DWORD);
WINAPI_CopyPointer pCopyFile = NULL;
WINAPI_MovePointer pMoveFileExW = NULL;

using std::ios;
using std::endl;
using std::ofstream;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:

            /*HookAPI hObj;
            hObj.HookFileCopy2();
            hObj.HookMoveFileExW();
            */
            HookFileCopy2();
            HookMoveFileExW();
            break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

BOOL userCopyFile(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, BOOL    bFailIfExists)
{
    std::wofstream fout("D:\\PRATHAMESH\\WINDOWS_API_HOOKING\\File_Monitoring\\FILE_RECORD\\file_copy_record.txt", ios::app);
    
    CString str1 = lpExistingFileName;
    CString str2 = lpNewFileName;

    std::wstring source(reinterpret_cast<const wchar_t *>(str1.GetString()));
    std::wstring destination(reinterpret_cast<const wchar_t *>(str2.GetString()));

    std::time_t now = std::time(0);
    #pragma warning(suppress : 4996)
    std::string str(std::ctime(&now));
    std::wstring date(str.begin(), str.end());
    
    fout << date << "[Source=" << source << "][Destination=" << destination << "]" << "\n";

    fout.close();

    BOOL result = pCopyFile(lpExistingFileName, lpNewFileName, bFailIfExists);

    return result;
}

void FileWrite(std::string str, DWORD result)
{
    char *errorMessage = nullptr;    
    
    if (0 != str.compare("GetProcAddress() Failed"))
    {
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, result,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errorMessage, 0, NULL);
    }

    ofstream fout("D:\\PRATHAMESH\\WINDOWS_API_HOOKING\\File_Monitoring\\FILE_RECORD\\api_hooking_log.txt", ios::app);

    fout << str << ":" << errorMessage << "------------------------------------------" << endl;

    fout.close();
}

void HookFileCopy2()
{
    DWORD result = 0l;

    pCopyFile = (WINAPI_CopyPointer)GetProcAddress(GetModuleHandle(L"KERNELBASE.DLL"), "CopyFile2");
    if (NULL == pCopyFile)
    { 
        FileWrite("GetProcAddress() Failed", result);
        return;
    }

    result = DetourTransactionBegin();
    FileWrite("TransactionBegin", result);

    result = DetourUpdateThread(GetCurrentThread());
    FileWrite("UpdateThread", result);

    result = DetourAttach(&(PVOID&)pCopyFile, userCopyFile);
    FileWrite("Attach", result);

    result = DetourTransactionCommit();
    FileWrite("TransactionCommit", result);
}

void HookMoveFileExW()
{
    DWORD result = 0l;

    pMoveFileExW = (WINAPI_MovePointer)GetProcAddress(GetModuleHandle(L"KERNELBASE.DLL"), "MoveFileExW");
    if (NULL == pMoveFileExW)
    {
        FileWrite("GetProcAddress() Failed", result);
        return;
    }

    result = DetourTransactionBegin();
    FileWrite("TransactionBegin", result);

    result = DetourUpdateThread(GetCurrentThread());
    FileWrite("UpdateThread", result);

    result = DetourAttach(&(PVOID&)pMoveFileExW, userMoveFileExW);
    FileWrite("Attach", result);

    result = DetourTransactionCommit();
    FileWrite("TransactionCommit", result);
}

BOOL userMoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD   dwFlags)
{
    BOOL result = false;
    std::wofstream fout("D:\\PRATHAMESH\\WINDOWS_API_HOOKING\\File_Monitoring\\FILE_RECORD\\file_delete_record.txt", ios::app);

    std::wstring source = lpExistingFileName;
    std::wstring destination = lpNewFileName;

    std::time_t now = std::time(0);
    #pragma warning(suppress : 4996)
    std::string str(std::ctime(&now));
    std::wstring date(str.begin(), str.end());
    date.erase(std::remove(date.begin(), date.end(), L'\n'), date.end());

    size_t pos = source.rfind(L"\\");
    std::wstring sub = source.substr(0, pos + 1);

    if (0 == destination.find(sub, 0))
        fout << date << "[Source=" << source << "][Destination=" << destination << "]" << "[RENAME]\n";
    else
        fout << date << "[Source=" << source << "][Destination=" << destination << "]" << "[DELETE]\n";
    fout.close();

    if(pMoveFileExW != NULL)
        result = pMoveFileExW(lpExistingFileName, lpNewFileName, dwFlags);

    return result;
}

