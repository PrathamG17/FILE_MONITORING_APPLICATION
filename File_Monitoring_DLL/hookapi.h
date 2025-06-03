/*#include <string>
#include <windows.h>

typedef BOOL(WINAPI* WINAPI_CopyPointer)(LPCTSTR, LPCTSTR, BOOL);
typedef BOOL(WINAPI* WINAPI_MovePointer)(LPCWSTR, LPCWSTR, DWORD);

class HookAPI
{
private :
    WINAPI_CopyPointer pCopyFile;
    WINAPI_MovePointer pMoveFileExW;
public:
    void HookFileCopy2();
    void HookMoveFileExW();
    void FileWrite(std::string, DWORD);
    BOOL userCopyFile(LPCTSTR, LPCTSTR, BOOL);
    BOOL userMoveFileExW(LPCWSTR, LPCWSTR, DWORD);
};
#pragma once
*/