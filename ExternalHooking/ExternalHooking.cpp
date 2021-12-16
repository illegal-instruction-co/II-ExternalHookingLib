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

#define NOMINMAX
#include "ExternalHooking.hpp"

int main() {
	
	SetConsoleTitleA("ILLEGAL INSTRUCTION CO");

	const auto instance = new II::ExternalHooking({
	"Notepad", // Window title or part of process name
	"C:\\Users\\PC\\source\\repos\\ExternalHooking\\x64\\Release\\ExampleDLL.dll"
	});

	bool status = instance->Setup();

	if (status) std::cout << "Successfully injected" << std::endl;
	if(!status) std::cout << "Failed to inject" << std::endl;
	
	uint64_t target  = (uint64_t)GetProcAddress(LoadLibraryA("User32.dll"), "MessageBoxA");
	uint64_t detour	 = (uint64_t)GetProcAddress(instance->m_image, "MessageBoxA_dtr");
	uint64_t pointer = (uint64_t)GetProcAddress(instance->m_image, "MessageBoxA_ptr");

	std::cout << "Target function:" << std::hex << target << std::endl;
	std::cout << "Detour function:" << std::hex << detour << std::endl;
	std::cout << "Pointer function:" << std::hex << pointer << std::endl;

	//instance->Hook(targetFunction, detour);
	instance->Target(target)->Detour(detour)->Pointer(pointer)->Hook();

	std::cin.clear();
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::cin.get();

	return 0;
}