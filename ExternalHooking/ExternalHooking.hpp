#pragma once
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

#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <vector>
#include <functional>
#include <thread>

using namespace std;

namespace II {

	struct handle_data {
		unsigned long process_id;
		HWND window_handle;
	};

	__forceinline BOOL is_main_window(HWND handle)
	{
		return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
	}

	__forceinline BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
	{
		handle_data& data = *(handle_data*)lParam;
		unsigned long process_id = 0;
		GetWindowThreadProcessId(handle, &process_id);
		if (data.process_id != process_id || !is_main_window(handle))
			return TRUE;
		data.window_handle = handle;
		return FALSE;
	}

	std::function<bool(HANDLE pHandle, DWORD pid, char buffer[MAX_PATH])> handleCb = {};

	class ExternalHooking {

	public:
		/*
		* Payload
		*/
		struct Payload {
			const char* process_identifier;
			const char* module_path;
		};

		struct ProcessInformation {
			DWORD PID;
			DWORD TID;
			HWND targetWindow;
			HMODULE injectedModule;
		};

		HMODULE m_image;
		ProcessInformation r_Inf;

	private: 
		Payload m_config;
		uint64_t m_targetFunc;
		uint64_t m_detourFunc;
		uint64_t m_pointerFunc;
		bool lastStep;

	private: 

		inline std::string as_string(std::string_view v) {
			return { v.data(), v.size() };
		}

		__forceinline DWORD GetProcessID() {
			std::wstring process_name_str = std::wstring(as_string(m_config.process_identifier).begin(), as_string(m_config.process_identifier).end());
			const wchar_t* processName = process_name_str.c_str();
			PROCESSENTRY32 processInfo;
			processInfo.dwSize = sizeof(processInfo);
			HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
			if (processesSnapshot == INVALID_HANDLE_VALUE) return 0;
			Process32First(processesSnapshot, &processInfo);
			if (wcsstr(processInfo.szExeFile, processName)) {
				CloseHandle(processesSnapshot);
				return (std::uint32_t)processInfo.th32ProcessID;
			}
			while (Process32Next(processesSnapshot, &processInfo))
			{
				if (wcsstr(processInfo.szExeFile, processName)) {
					CloseHandle(processesSnapshot);
					return (std::uint32_t)processInfo.th32ProcessID;
				}
			}
			CloseHandle(processesSnapshot);
			return 0x00;
		}

		__forceinline HWND FindMainWindow(unsigned long process_id)
		{
			handle_data data;
			data.process_id = process_id;
			data.window_handle = 0;
			EnumWindows(enum_windows_callback, (LPARAM)&data);
			return data.window_handle;
		}

		__forceinline HWND FindTargetWindow(DWORD processId) {
			HWND hwnd = FindMainWindow(processId);
			if (hwnd == NULL) {
				return 0x0;
			}
			else {
				return hwnd;
			}
		}

		__forceinline bool LoadImageC() {
			m_image = LoadLibraryExA(m_config.module_path, NULL, DONT_RESOLVE_DLL_REFERENCES);
			if (!m_image) return false;
			return true;
		}

		__forceinline static auto HandleReceiver(HANDLE* io_port) {
			DWORD nOfBytes;
			ULONG_PTR cKey;
			LPOVERLAPPED pid;
			char buffer[MAX_PATH];
			while (GetQueuedCompletionStatus(*io_port, &nOfBytes, &cKey, &pid, -1))
				if (nOfBytes == 6) {
					HANDLE race_handle = OpenProcess(PROCESS_ALL_ACCESS, true, (DWORD)pid);
					GetModuleFileNameExA(race_handle, 0, buffer, MAX_PATH);
					return handleCb(race_handle, (DWORD)pid, buffer);
				}
		}

		__forceinline bool NotStrippedHandle(std::function<bool(HANDLE pHandle, DWORD pid, char buffer[MAX_PATH])> callback) {
			handleCb = callback;
			auto exp_handle = OpenProcess(PROCESS_ALL_ACCESS, true, r_Inf.PID);
			auto io_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
			auto job_object = CreateJobObjectW(0, 0);
			auto job_io_port = JOBOBJECT_ASSOCIATE_COMPLETION_PORT{ 0, io_port };
			auto result = SetInformationJobObject(job_object, JobObjectAssociateCompletionPortInformation, &job_io_port, sizeof(job_io_port));
			result = AssignProcessToJobObject(job_object, exp_handle);
			auto threadHandle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)HandleReceiver, &io_port, 0, 0);
			WaitForSingleObject(threadHandle, -1);
			CloseHandle(exp_handle);
			return true;
		}

		__forceinline bool LoadLibraryBypasses() {
			NotStrippedHandle([&](HANDLE pHandle, DWORD pid, char buffer[MAX_PATH]) {
				if (pid == r_Inf.PID) {

					/*
					* TO DO 
					* Improve this shit!
					*/
					uint64_t LdrLoadDllAddr = (uint64_t)GetProcAddress(LoadLibraryA("ntdll.dll"), "LdrLoadDll");
					BYTE LdrLoadDllShell[] = { 0x48, 0x89, 0x5C, 0x24, 0x10 };
					return WriteProcessMemory(pHandle, (LPVOID)LdrLoadDllAddr, &LdrLoadDllShell, sizeof(LdrLoadDllShell), 0);
				}
			});
			return true;
		}

		__forceinline bool InjectDLL() {
			HOOKPROC addr = (HOOKPROC)GetProcAddress(m_image, "NextHook");
			SetWindowsHookExW(WH_GETMESSAGE, addr, m_image, r_Inf.TID);
			return PostThreadMessageW(r_Inf.TID, WM_NULL, NULL, NULL);
		}

		__forceinline uintptr_t GetRemoteModuleBaseAddress(DWORD procId, const wchar_t* modName)
		{
			uintptr_t modBaseAddr = 0;
			HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
			if (hSnap != INVALID_HANDLE_VALUE)
			{
				MODULEENTRY32 modEntry;
				modEntry.dwSize = sizeof(modEntry);
				if (Module32First(hSnap, &modEntry))
				{
					do
					{
						if (!_wcsicmp(modEntry.szModule, modName))
						{
							modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
							break;
						}
					} while (Module32Next(hSnap, &modEntry));
				}
			}
			CloseHandle(hSnap);
			return modBaseAddr;
		}

		__forceinline bool HookC(uint64_t addr, uint64_t hookAddr) {

			bool result = NotStrippedHandle([&](HANDLE pHandle, DWORD pid, char buffer[MAX_PATH]) {
				if (pid == r_Inf.PID) {

					/*
					* The hook jmp takes 5 bytes
					*/
					int m_len = 5;


					DWORD relativeAddy;

					/*
					* Just some variables needed to apply the hook
					*/
					BYTE jump = 0xE9;
					BYTE NOP = 0x90;
					BYTE NOPS[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

					/*
					* Calculate the jump to the hook function
					*/
					relativeAddy = (hookAddr - addr) - 5;
					/*
					* Write the jump
					*/
					bool result_jump = WriteProcessMemory(pHandle, (LPVOID)addr, &jump, sizeof(jump), NULL);
					bool result_relative = WriteProcessMemory(pHandle, (LPVOID)(addr + 0x1), &relativeAddy, sizeof(relativeAddy), NULL);
					/*
					* Write all of the nops at once, instead of making multiple calls
					*/
					int new_len = m_len - 5;
					if (new_len > 0)
					{
						WriteProcessMemory(pHandle, (LPVOID)(addr + 0x5), NOPS, new_len, NULL);
					}

					std::cout << "START" << std::endl;
					std::cout << result_jump << std::endl;
					std::cout << result_relative << std::endl;
					std::cout << "END" << std::endl;

					return true;
				}
				});

			return result;
		}

	public:
		ExternalHooking(Payload config) {
			m_config = config;
			r_Inf.PID = this->GetProcessID();
			if (!r_Inf.PID) {
				r_Inf.targetWindow = FindWindowA(NULL, m_config.process_identifier);
				r_Inf.TID = GetWindowThreadProcessId(r_Inf.targetWindow, &r_Inf.PID);
			} else {
				r_Inf.targetWindow = FindTargetWindow(r_Inf.PID);
				r_Inf.TID = GetWindowThreadProcessId(r_Inf.targetWindow, nullptr);
			}
		}

		__forceinline bool Setup() {
			if (!m_config.process_identifier) return false;
			if (!r_Inf.PID) return false;
			if (!r_Inf.targetWindow) return false;
			if (!r_Inf.TID) return false;
			if (!this->LoadImageC()) return false;
			if (!this->LoadLibraryBypasses()) return false;
			if (!this->InjectDLL()) return false;

			return true;
		}

		__forceinline ExternalHooking * Target(uint64_t func) {
			m_targetFunc = func;
			lastStep = true;
			return this;
		}

		__forceinline ExternalHooking * Detour(uint64_t func) {
			m_detourFunc = func;
			lastStep = true;
			return this;
		}

		__forceinline ExternalHooking * Pointer(uint64_t func) {
			m_pointerFunc = func;
			lastStep = HookC(m_pointerFunc, m_targetFunc);
			return this;
		}

		__forceinline bool Hook() {
			if(lastStep) this->HookC(m_targetFunc, m_detourFunc);
			return false;
		}


	};

}