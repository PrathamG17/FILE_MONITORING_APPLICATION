#pragma once
#include <string>
#include <windows.h>

typedef BOOL(WINAPI* WINAPI_CopyPointer)(LPCTSTR, LPCTSTR, BOOL);
typedef BOOL(WINAPI* WINAPI_MovePointer)(LPCWSTR, LPCWSTR, DWORD);

class HookAPI
{
private :
    static WINAPI_CopyPointer pCopyFile;
    static WINAPI_MovePointer pMoveFileExW;
public:
    ~HookAPI();
    void HookFileAPI();
private:
    void apiLog(std::string, DWORD);
    static BOOL userCopyFile(LPCTSTR, LPCTSTR, BOOL);
    static BOOL userMoveFileExW(LPCWSTR, LPCWSTR, DWORD);
};
