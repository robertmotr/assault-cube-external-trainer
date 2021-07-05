#include <iostream>
#include <vector>
#include <Windows.h>
#include <tlhelp32.h>
#include <string>
#include <wchar.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion-null"

using namespace std;

/* 	This struct is a reference for the offsets of important values I've determined in the playerEntity object using cheat engine.
	In order to get to this object in memory, I've found that that the base address of the module + offset 0x00109B74 holds a pointer to the struct.
	So far, I've found this offset pretty reliable and I would recommend using this.
	TODO: add constructor to struct allowing pointer to playerObject, let the constructor determine offsets for all the values
/**/
struct playerEntity {
	float x;	// 0x34
	float y;	// 0x38
	float z;	// 0x3C

	float yaw;	// 0x40
	float pitch;	// 0x44

	unsigned int hp;	// 0xF8
	unsigned int armour;	// 0xFC

	unsigned int sWeaponReserve; 	// 0x114
	unsigned int pWeaponReserve; 	// 0x128

	unsigned int sWeapon;	// 0x13C
	unsigned int pWeapon;	// 0x150
};

// Error occured while (stuff here):\n GetLastError()
void displayError(string typefailure) {
	cout << "Error occurred while " + typefailure + ":\n"<< GetLastError() << endl;
	system("PAUSE");
}

void displayInstructions() {
		cout << "[Numpad 0]	Enable god mode" << endl;
		cout << "[Numpad 1]	Enable infinite primary weapon ammo" << endl;
		cout << "[Numpad 2]	Enable infinite secondary weapon ammo" << endl;
		cout << "[Numpad 3]	Enable infinite armour" << endl;
		cout << "[Numpad 4]	Teleport using X, Y and Z co-ordinates" << endl;
}

void godMode(const uintptr_t &health, const HANDLE &handleProcess) {
	int newHealth = 999999;
	WriteProcessMemory(handleProcess, (LPVOID) health, &newHealth, sizeof(newHealth), NULL);

	cout << endl;
	cout << "Enabled god mode." << endl;
	Sleep(3000);
	system("CLS");
	displayInstructions();
}

void infinitePrimary(const uintptr_t &pWeapon, const uintptr_t &pReserve, const HANDLE &handleProcess) {
	int newAmmo = 99999;
	WriteProcessMemory(handleProcess, (LPVOID) pWeapon, &newAmmo, sizeof(newAmmo), NULL);
	WriteProcessMemory(handleProcess, (LPVOID) pReserve, &newAmmo, sizeof(newAmmo), NULL);

	cout << endl;
	cout << "Enabled infinite primary ammo." << endl;
	Sleep(3000);
	system("CLS");
	displayInstructions();
}

void infiniteSecondary(const uintptr_t &sWeapon, const uintptr_t &sReserve, const HANDLE &handleProcess) {
	int newAmmo = 99999;
	WriteProcessMemory(handleProcess, (LPVOID) sWeapon, &newAmmo, sizeof(newAmmo), NULL);
	WriteProcessMemory(handleProcess, (LPVOID) sReserve, &newAmmo, sizeof(newAmmo), NULL);

	cout << endl;
	cout << "Enabled infinite secondary ammo." << endl;
	Sleep(3000);
	system("CLS");
	displayInstructions();
}

void infiniteArmour(const uintptr_t &armour, const HANDLE &handleProcess) {
	int newArmour = 99999;
	WriteProcessMemory(handleProcess, (LPVOID) armour, &newArmour, sizeof(newArmour), NULL);

	cout << endl;
	cout << "Enabled infinite armour." << endl;
	Sleep(3000);
	system("CLS");
	displayInstructions();
}

void teleport(const uintptr_t &px, const uintptr_t &py, const uintptr_t &pz, const HANDLE &handleProcess) {
	system("CLS");
	cout << "Enter the co-ordinates separated by spaces you'd like to teleport to." << endl;
	int x, y, z; 
	cin >> x >> y >> z;

	WriteProcessMemory(handleProcess, (LPVOID) px, &x, sizeof(x), NULL);
	WriteProcessMemory(handleProcess, (LPVOID) py, &y, sizeof(y), NULL);
	WriteProcessMemory(handleProcess, (LPVOID) pz, &z, sizeof(z), NULL);

	cout << endl;
	cout << "Teleported to location." << endl;
	Sleep(3000);
	system("CLS");
	displayInstructions();
}

DWORD GetProcId(const CHAR* processName){
    // https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot
	DWORD procid = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hSnap == NULL) {
		displayError("trying to get snapshot of processes on GetProcId");
	}

	else {
		// https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/ns-tlhelp32-processentry32
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(pe32); // microsoft wants us to declare size of this struct in dwSize variable

		// call proc32first to get first process off of snapshot, then proc32next afterwards
        if(Process32First(hSnap, &pe32) == FALSE) {
            displayError("calling Process32First in GetProcId()");
        }
        else {
			do {
				if(_strcmpi(pe32.szExeFile, processName) == 0) {
					procid = pe32.th32ProcessID;
					break;
				}
			}
			while(Process32Next(hSnap, &pe32));
        }
	}
	CloseHandle(hSnap);
	return procid;
}

// default for executables is 0x400000, use this if ASLR is turned on or you don't know the module base
// returns base address of module given process id. accepts process id, and windows wide string of module name.
uintptr_t GetModuleBaseAddress(DWORD processId, const CHAR* modName) {
	uintptr_t moduleBaseAddress = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
	if(hSnap == NULL) {
		displayError("trying to get snapshot on GetModBaseAddr for modules");
	}
	else {
		MODULEENTRY32 me;
		me.dwSize = sizeof(MODULEENTRY32);

		if(Module32First(hSnap, &me)) {
			do {
				if(_strcmpi(me.szModule, modName) == 0) {
					moduleBaseAddress = (uintptr_t) me.modBaseAddr;
					break;
				}
			}
			while (Module32Next(hSnap, &me));
		}
	}
	CloseHandle(hSnap);
	return moduleBaseAddress;
}

// openprocess handle, baseptr, offsets as vector. use this for traversing pointer chains with offsets
// haven't used this yet, but i'm keeping it here because it's extremely helpful in the event that i need it.
uintptr_t findAddress(HANDLE hProc, uintptr_t ptr, vector<unsigned int> offsets) {
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i) {
		ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
		addr += offsets[i];
	}
	return addr;
}

int main() {

	DWORD process = GetProcId("ac_client.exe");
	// cast dword to uint just in case it doesnt play well
	cout << "Process ID retrieved: " << (unsigned int) process << endl; 
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, process);
	
	if(hProcess == NULL) {
		displayError("calling OpenProcess() in main()");
		system("PAUSE");
	}
	else {
		cout << "Got handle from process" << endl;
		uintptr_t moduleBase = GetModuleBaseAddress(process, "ac_client.exe");
		cout << "Got base address of module: 0x" << hex << moduleBase << endl;

		uintptr_t playerObject = moduleBase + 0x00109B74;

		// interesting stuff here, sizeof(unsigned int) on third parameter of this function was returning 6 bytes which caused me a few hours of frustration.	
		// changing it to flat out four bytes fixed it. very strange!
		if(ReadProcessMemory(hProcess, (BYTE*) playerObject, &playerObject, 4, NULL) == 0) {
			displayError("calling RPM in main() to resolve player object offset");
		}
		else {
			uintptr_t x = playerObject + 0x34;
			uintptr_t y = playerObject + 0x38;
			uintptr_t z = playerObject + 0x3C;

			uintptr_t health = playerObject + 0xF8;
			uintptr_t armour = playerObject + 0xFC;

			uintptr_t secondaryWeaponReserve = playerObject + 0x114;
			uintptr_t secondaryWeapon = playerObject + 0x13C;

			uintptr_t primaryWeaponReserve = playerObject + 0x128;
			uintptr_t primaryWeapon = playerObject + 0x150;

			cout << "Menu has successfully started." << endl;
			cout << endl;
			Sleep(3000);
			system("CLS");
			displayInstructions();

			vector<unsigned int> virtualKeys = {VK_NUMPAD0, VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD4};
			while(true) {
				for(int i = 0; i < virtualKeys.size(); i++) {
					// https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getkeystate
					// if short return value has MSB of 1, then key is down. use bitwise AND to determine if this is true
					if(GetKeyState(virtualKeys[i]) & 0x8000) {
						if(i == 0) {
							godMode(health, hProcess);
						}
						else if(i == 1) {
							infinitePrimary(primaryWeapon, primaryWeaponReserve, hProcess);
						}
						else if(i == 2) {
							infiniteSecondary(secondaryWeapon, secondaryWeaponReserve, hProcess);

						}
						else if(i == 3) {
							infiniteArmour(armour, hProcess);
						}
						else {
							teleport(x, y, z, hProcess);
						}
					}
				}
			}
		}
	}

	CloseHandle(hProcess);
	return 0;
}