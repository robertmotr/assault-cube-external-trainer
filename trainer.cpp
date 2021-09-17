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


//use template to save time and avoid potential bug
template <typename T>
void WriteMem(HANDLE handleProcess, LPVOID address, const T& value)
{
	WriteProcessMemory(handleProcess, address, &value, sizeof(value), NULL);
}

//why do I make struct?
//because Windows Memory reading/writing functions are VERY expensive
//writing a chunk of data at once efficiently speed your chear hax
//https://www.unknowncheats.me/forum/general-programming-and-reversing/347717-rpm-benchmark-data-size-matter.html
struct Position
{
	float x;
	float y;
	float z;
};

struct Angle
{
	//a bit strange, because most of the video games have orders in pitch -> yaw -> roll
	//pitch vertical angle, yaw horizontal angle
	float yaw;
	float pitch;
	//float roll
};

struct playerEntity
{
	/*
	float x;
	float y;
	float z;
	*/
	Position pos;// 0x34


	/*
	float yaw;
	float pitch;
	*/
	Angle angle; // 0x40

	unsigned int hp;	 // 0xF8
	unsigned int armour; // 0xFC

	unsigned int sWeaponReserve; // 0x114
	unsigned int pWeaponReserve; // 0x128

	unsigned int sWeapon; // 0x13C
	unsigned int pWeapon; // 0x150
};

// Error occured while (stuff here):\n GetLastError()

//you seriously want to copy the whole string?
//void displayError(string typefailure)
void displayError(const string& typefailure)
{
	cout << "Error occurred while " + typefailure + ":\n"
		 << GetLastError() << endl;
	system("PAUSE");
}

void displayInstructions()
{
	cout << "[Numpad 0]	Enable god mode\n";// << endl;
	cout << "[Numpad 1]	Enable infinite primary weapon ammo\n";// << endl;
	cout << "[Numpad 2]	Enable infinite secondary weapon ammo\n";// << endl;
	cout << "[Numpad 3]	Enable infinite armour\n";// << endl;
	cout << "[Numpad 4]	Teleport using X, Y and Z co-ordinates" << endl;
}

//uintptr_t size is 4 byte, while pointer may be 4/8 bytes. 
//However, dereference the pointer costs a few more instructions, so copy value is faster
void godMode(uintptr_t healthAddr, HANDLE handleProcess)
{
	int newHealth = 999999;

	//template time!
	WriteMem(handleProcess, healthAddr, newHealth);
	//WriteProcessMemory(handleProcess, (LPVOID)health, &newHealth, sizeof(newHealth), NULL);

	//endl flushes the buffer for no reason
	//https://www.hellocodies.com/endl-and-n/
	//cout << endl;
	cout << "\nEnabled god mode." << endl;
	Sleep(3000);
	system("CLS");
	displayInstructions();
}

void infinitePrimary(uintptr_t pWeaponAddr, uintptr_t pReserveAddr, HANDLE handleProcess)
{
	int newAmmo = 99999;
	WriteMem(handleProcess, pWeaponAddr, newAmmo);
	WriteMem(handleProcess, pReserveAddr, newAmmo);
	//WriteProcessMemory(handleProcess, (LPVOID)pWeapon, &newAmmo, sizeof(newAmmo), NULL);
	//WriteProcessMemory(handleProcess, (LPVOID)pReserve, &newAmmo, sizeof(newAmmo), NULL);


	//please stop abusing endl 
	//cout << endl;
	cout << "\nEnabled infinite primary ammo." << endl;
	Sleep(3000);
	system("CLS");
	displayInstructions();
}

void infiniteSecondary(uintptr_t sWeaponAddr, uintptr_t sReserveAddr, HANDLE handleProcess)
{
	int newAmmo = 99999;
	WriteMem(handleProcess, sWeaponAddr, newAmmo);
	WriteMem(handleProcess, sReserveAddr, newAmmo);
	//WriteProcessMemory(handleProcess, (LPVOID)sWeapon, &newAmmo, sizeof(newAmmo), NULL);
	//WriteProcessMemory(handleProcess, (LPVOID)sReserve, &newAmmo, sizeof(newAmmo), NULL);

	//cout << endl;
	cout << "\nEnabled infinite secondary ammo." << endl;
	Sleep(3000);
	system("CLS");
	displayInstructions();
}

void infiniteArmour(uintptr_t armourAddr, HANDLE handleProcess)
{
	int newArmour = 99999;
	WriteMem(handleProcess, armourAddr, newArmour);
	//WriteProcessMemory(handleProcess, (LPVOID)armour, &newArmour, sizeof(newArmour), NULL);

	//cout << endl;
	cout << "\nEnabled infinite armour." << endl;
	Sleep(3000);
	system("CLS");
	displayInstructions();
}

void teleport(uintptr_t positionAddr, HANDLE handleProcess)
{
	system("CLS");
	cout << "Enter the co-ordinates separated by spaces you'd like to teleport to." << endl;
	Position pos;
	cin >> pos.x >> pos.y >> pos.z;

	WriteMem(handleProcess, positionAddr, pos);
	//WriteProcessMemory(handleProcess, (LPVOID)px, &x, sizeof(x), NULL);
	//WriteProcessMemory(handleProcess, (LPVOID)py, &y, sizeof(y), NULL);
	//WriteProcessMemory(handleProcess, (LPVOID)pz, &z, sizeof(z), NULL);

	//cout << endl;
	cout << "\nTeleported to location." << endl;
	Sleep(3000);
	system("CLS");
	displayInstructions();
}

//seriously?
//DWORD GetProcId(const CHAR *processName)

//even this is better -> DWORD GetProcId(const string& processName), you can then use processName.c_str()
DWORD GetProcId(const char* processName)
{
	// https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot
	DWORD procid = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == NULL)
	{
		displayError("trying to get snapshot of processes on GetProcId");
	}

	else
	{
		// https://docs.microsoft.com/en-us/windows/win32/api/tlhelp32/ns-tlhelp32-processentry32
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(pe32); // microsoft wants us to declare size of this struct in dwSize variable

		// call proc32first to get first process off of snapshot, then proc32next afterwards
		if (Process32First(hSnap, &pe32) == FALSE)
		{
			displayError("calling Process32First in GetProcId()");
		}
		else
		{
			do
			{
				if (_strcmpi(pe32.szExeFile, processName) == 0)
				{
					procid = pe32.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &pe32));
		}
	}
	CloseHandle(hSnap);
	return procid;
}

// default for executables is 0x400000, use this if ASLR is turned on or you don't know the module base
// returns base address of module given process id. accepts process id, and windows wide string of module name.
//uintptr_t GetModuleBaseAddress(DWORD processId, const CHAR *modName)
uintptr_t GetModuleBaseAddress(DWORD processId, const char *modName)
{
	uintptr_t moduleBaseAddress = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
	if (hSnap == NULL)
	{
		displayError("trying to get snapshot on GetModBaseAddr for modules");
	}
	else
	{
		MODULEENTRY32 me;
		me.dwSize = sizeof(MODULEENTRY32);

		if (Module32First(hSnap, &me))
		{
			do
			{
				if (_strcmpi(me.szModule, modName) == 0)
				{
					moduleBaseAddress = (uintptr_t)me.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &me));
		}
	}
	CloseHandle(hSnap);
	return moduleBaseAddress;
}

// openprocess handle, baseptr, offsets as vector. use this for traversing pointer chains with offsets
// haven't used this yet, but i'm keeping it here because it's extremely helpful in the event that i need it.
uintptr_t findAddress(HANDLE hProc, uintptr_t ptr, vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		//i don't understand why did you cast the uintptr_t to BYTE*, which then later converted to LPVOID by the compiler again....
		ReadProcessMemory(hProc, (BYTE *)addr, &addr, sizeof(addr), 0);
		addr += offsets[i];
	}
	return addr;
}

int main()
{
	//I SWEAR IF YOU KEEP FLUSHING BUFFER FOR NO REASON >:(
	cout << "This cheat for Assault Cube was made by Robert Motrogeanu for educational purposes and its contents is published freely on my GitHub.\n";// << endl;
	cout << "github.com/robertmotr\n" << endl;
	//cout << endl;

	DWORD process = GetProcId("ac_client.exe");
	// cast dword to uint just in case it doesnt play well
	cout << "Process ID retrieved: " << (unsigned int)process << endl;
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, process);

	if (hProcess == NULL)
	{
		displayError("calling OpenProcess() in main()");
		system("PAUSE");
	}
	else
	{
		cout << "Got handle from process" << endl;
		uintptr_t moduleBase = GetModuleBaseAddress(process, "ac_client.exe");
		cout << "Got base address of module: 0x" << hex << moduleBase << endl;

		uintptr_t playerObjectBaseAddr = moduleBase + 0x00109B74;

		// interesting stuff here, sizeof(unsigned int) on third parameter of this function was returning 6 bytes which caused me a few hours of frustration.
		// changing it to flat out four bytes fixed it. very strange!
		if (ReadProcessMemory(hProcess, (BYTE *)playerObjectBaseAddr, &playerObjectBaseAddr, sizeof(playerObjectBaseAddr), NULL) == 0)
		{
			displayError("calling RPM in main() to resolve player object offset");
		}
		else
		{
			uintptr_t posAddr = playerObjectBaseAddr + 0x34;

			uintptr_t healthAddr = playerObjectBaseAddr + 0xF8;
			uintptr_t armourAddr = playerObjectBaseAddr + 0xFC;

			uintptr_t secondaryWeaponReserveAddr = playerObjectBaseAddr + 0x114;
			uintptr_t secondaryWeaponAddr = playerObjectBaseAddr + 0x13C;

			uintptr_t primaryWeaponReserveAddr = playerObjectBaseAddr + 0x128;
			uintptr_t primaryWeaponAddr = playerObjectBaseAddr + 0x150;

			cout << "Menu has successfully started.\n" << endl;
			//cout << endl;
			Sleep(3000);
			system("CLS");
			displayInstructions();

			while (true)
			{
				// https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getkeystate
				// if short return value has MSB of 1, then key is down. use bitwise AND to determine if this is true
				if(GetAsyncKeyState(VK_NUMPAD0) & 0x8000) {
					godMode(healthAddr, hProcess);
				}
				else if(GetAsyncKeyState(VK_NUMPAD1) & 0x8000) {
					infinitePrimary(primaryWeaponAddr, primaryWeaponReserveAddr, hProcess);
				}
				else if(GetAsyncKeyState(VK_NUMPAD2) & 0x8000) {
					infiniteSecondary(secondaryWeaponAddr, secondaryWeaponReserveAddr, hProcess);
				}
				else if(GetAsyncKeyState(VK_NUMPAD3) & 0x8000) {
					infiniteArmour(armourAddr, hProcess);
				}
				else if(GetAsyncKeyState(VK_NUMPAD4) & 0x8000) {
					teleport(posAddr, hProcess);
				}
			}
		}
	}

	CloseHandle(hProcess);
	return 0;
}