#include "pch.h"
#include <string>
#include <fstream>
#include <atlstr.h>
#include <iostream>
#include <winbase.h>
#include "detours.h"
#include "hookapi.h"

WINAPI_CopyPointer HookAPI::pCopyFile = NULL;
WINAPI_MovePointer HookAPI::pMoveFileExW = NULL;

HookAPI::~HookAPI()
{
    pCopyFile = NULL;
    pMoveFileExW = NULL;
}
void HookAPI::HookFileAPI()
{
    DWORD result = 0l;

    pCopyFile = (WINAPI_CopyPointer)GetProcAddress(GetModuleHandle(L"KERNELBASE.DLL"), "CopyFile2");
    if (NULL == pCopyFile)
    {
        apiLog("GetProcAddress() Failed", result);
        return;
    }

    pMoveFileExW = (WINAPI_MovePointer)GetProcAddress(GetModuleHandle(L"KERNELBASE.DLL"), "MoveFileExW");
    if (NULL == pMoveFileExW)
    {
        apiLog("GetProcAddress() Failed", result);
        return;
    }

    result = DetourTransactionBegin();
    apiLog("TransactionBegin", result);

    result = DetourUpdateThread(GetCurrentThread());
    apiLog("UpdateThread", result);

    result = DetourAttach(&(PVOID&)pCopyFile, (PVOID)&HookAPI::userCopyFile);
    apiLog("AttachCopyFile", result);

    result = DetourAttach(&(PVOID&)pMoveFileExW, (PVOID)&HookAPI::userMoveFileExW);
    apiLog("AttachMoveFileExW", result);

    result = DetourTransactionCommit();
    apiLog("TransactionCommit", result);
}
BOOL HookAPI::userMoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD   dwFlags)
{
    BOOL result = false;
    std::wofstream fout("D:\\PRATHAMESH\\FILE_MONITORING_APPLICATION\\File_Monitoring\\FILE_LOGS\\file_operation_logs.txt", std::ios::app);

    std::wstring source = lpExistingFileName;
    std::wstring destination = lpNewFileName;

    std::time_t now = std::time(0);
    #pragma warning(suppress : 4996)
    std::string str(std::ctime(&now));
    std::wstring date(str.begin(), str.end());
    date.erase(std::remove(date.begin(), date.end(), L'\n'), date.end());

    size_t pos = source.rfind(L"\\");
    std::wstring sub = source.substr(0, pos + 1);

    pos = destination.rfind(L"\\");
    std::wstring sub1 = destination.substr(0, pos + 1);

    if (0 == sub1.compare(sub))
        fout << date << "[Source=" << source << "][Destination=" << destination << "]" << "[RENAME]\n";
    else if (destination.npos != destination.find(L"$RECYCLE.BIN"))
        fout << date << "[Source=" << source << "][Destination=" << destination << "]" << "[DELETE]\n";
    else
        fout << date << "[Source=" << source << "][Destination=" << destination << "]" << "[MOVE]\n";

    fout.close();

    if (pMoveFileExW != NULL)
        result = pMoveFileExW(lpExistingFileName, lpNewFileName, dwFlags);

    return result;
}

BOOL HookAPI::userCopyFile(LPCTSTR lpExistingFileName, LPCTSTR lpNewFileName, BOOL    bFailIfExists)
{
    BOOL result = false;

    std::wofstream fout("D:\\PRATHAMESH\\FILE_MONITORING_APPLICATION\\File_Monitoring\\FILE_LOGS\\file_operation_logs.txt", std::ios::app);

    CString str1 = lpExistingFileName;
    CString str2 = lpNewFileName;

    std::wstring source(reinterpret_cast<const wchar_t*>(str1.GetString()));
    std::wstring destination(reinterpret_cast<const wchar_t*>(str2.GetString()));

    std::time_t now = std::time(0);
    #pragma warning(suppress : 4996)
    std::string str(std::ctime(&now));
    std::wstring date(str.begin(), str.end());
    date.erase(std::remove(date.begin(), date.end(), L'\n'), date.end());

    fout << date << "[Source=" << source << "][Destination=" << destination << "][COPY]\n";

    fout.close();

    if(pCopyFile != NULL)
        result = pCopyFile(lpExistingFileName, lpNewFileName, bFailIfExists);

    return result;
}
void HookAPI::apiLog(std::string str, DWORD result)
{
    char* errorMessage = NULL;

    if (0 != str.compare("GetProcAddress() Failed"))
    {
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, result,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&errorMessage, 0, NULL);
    }

    std::ofstream fout("D:\\PRATHAMESH\\FILE_MONITORING_APPLICATION\\File_Monitoring\\FILE_LOGS\\api_hooking_logs.txt", std::ios::app);

    if (errorMessage != NULL)
    {
        std::string error = std::string(errorMessage);
        error.erase(std::remove_if(error.begin(), error.end(), [](char c) { return c == '\n' || c == '\r'; }), error.end());

        if (0 == str.compare("TransactionBegin"))
        {
            std::time_t now = std::time(0);
            #pragma warning(suppress : 4996)
            std::string date(std::ctime(&now));
            date.erase(std::remove(date.begin(), date.end(), L'\n'), date.end());

            fout << "\n" + date + "[" + str + ":" + error + "]";
        }
        else
            fout << "[" + str + ":" + error + "]";

        fout.close();
    }
}
