/*
* Copyright 2021 ILLEGAL-INSTRUCTION-CO
*
* I am too lazy to find a suitable license for this app.
* You can do whatever you want with this app and you don't have to mention my name.
* Life is really hard, and sometimes we need to make it easy for each other. Bye babe.
* -----------------
* MBK
* -----------------
*/
#include <windows.h>
#include <iostream>

void main() {
    MessageBoxExA(NULL, "MAIN", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST, 0);

    Sleep(500);
    MessageBoxA(NULL, "TEST", "Msg title", MB_OK | MB_ICONQUESTION);
}

// Exporting function usable with SetWindowsHookEx
extern "C" __declspec(dllexport) int NextHook(int code, WPARAM wParam, LPARAM lParam) {
    return CallNextHookEx(NULL, code, wParam, lParam);
}

// Exporting hook function
extern "C" __declspec(dllexport) INT WINAPI HookedMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
    MessageBoxExW(NULL, L"I HOOKED YOU BITCH", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) main();
    return TRUE;
}

