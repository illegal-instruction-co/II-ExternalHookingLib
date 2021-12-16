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
    MessageBoxExA(NULL, "Injected", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST, 0);

    /*
    * MessageBox a will be hooked!
    */
    MessageBoxA(NULL, "IF I HOOK HERE, THIS TEXT WILL BE REPLACED WITH ONLY \\MBK\\", "Msg title", MB_OK | MB_ICONQUESTION);
}

/*
* Exporting function usable with SetWindowsHookEx
*/
extern "C" __declspec(dllexport) int NextHook(int code, WPARAM wParam, LPARAM lParam) {
    return CallNextHookEx(NULL, code, wParam, lParam);
}

/*
* Exporting pointer
*/
/*
* Exporting detour function
*/
extern "C" __declspec(dllexport) INT WINAPI MessageBoxA_ptr(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
   /*
   * this babe will be a pointer
   * when she grows up
   */
    MessageBoxExA(NULL, "POINTER ISNT AN ADULT ?", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST, 0);
    return 0;
}

/*
* Exporting detour function
*/
extern "C" __declspec(dllexport) INT WINAPI MessageBoxA_dtr(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
   MessageBoxExA(NULL, "HOOKED ?", NULL, MB_OK | MB_ICONERROR | MB_APPLMODAL | MB_SETFOREGROUND | MB_TOPMOST, 0);
   lpText = L"MBK";
   return MessageBoxA_ptr(hWnd, lpText, lpCaption, uType);
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) main();
    return TRUE;
}

