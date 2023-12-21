/* Hello there! Any mistake or any good change that you may be notice,
 * please report to me, enjoy the code and try improve it, thank you!!! 
 */

#include <windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <fstream>

void display_ini_msg(); // function prototype: display the initial message

/* inject_dll(): the function thats inject our DLL */
int inject_dll(const char *dll_path, std::wstring procName)
{
	const wchar_t* pName = procName.c_str(); // set procName to wchar_t* type
	HANDLE snapshot = 0;                     // initialize 'snapshot' variable 
	PROCESSENTRY32 pe32 = { 0 };             // initialize 'pe32' struct object
	DWORD exitCode = 0;						 // initialize exitCode variable
	std::wstring process_found;              // Holds the process name found

	pe32.dwSize = sizeof(PROCESSENTRY32); // set the 'dwSize' parameter

	// Take a snapshot of all processes in the system
	snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE)
	{
		std::cout << "\nERROR: snapshot variable receives a Invalid Handle Value.\n";
		return -1;
	}

	// Retrieves information about the first process encountered in a system snapshot
	Process32First(snapshot, &pe32);

	do {
		// Try to find the process name by iterating through the 'pe32' object
		if (wcscmp(pe32.szExeFile, pName) == 0)
		{
			process_found = pe32.szExeFile;

			// get a Process Handle to the program
			HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, true, pe32.th32ProcessID);
			if (process == NULL)
			{
				std::cout << "\nERROR: OpenProcess return NULL.\n";
				return -1;
			}

			// Alocate memory in the current process to hold our Dll path
			void* lpBaseAddress = VirtualAllocEx(process, NULL, strlen(dll_path) + 1, MEM_COMMIT, PAGE_READWRITE);

			// Checks if the allocation as successfully
			if (lpBaseAddress == 0)
			{
				std::cout << "\nERROR: VirtualAllocEx is zero.\n";
				return -1; 
			}

			// Write the Dll path in the memory allocated and checks if it was successfully
			if (WriteProcessMemory(process, lpBaseAddress, dll_path, strlen(dll_path) + 1, NULL) == 0)
			{
				std::cout << "\nERROR: VirtualAllocEx is zero.\n";
				return -1;
			}

			// Create a remote thread inside the process
			HMODULE kernel32base = GetModuleHandle(L"kernel32.dll");
			if(kernel32base == 0) // Checks if the "kernel32base" variable is not zero
			{
				std::cout << "\nERROR: GetModuleHandle filled kernel32base variable with zero.\n";
				return -1;
			}
			HANDLE thread = CreateRemoteThread(process, NULL, 0, 
				                               (LPTHREAD_START_ROUTINE)GetProcAddress(kernel32base, "LoadLibraryA"),
				                 			   lpBaseAddress, 0, NULL);
			// Checks if the "thread" variable is not zero
			if (thread == 0)
			{
				std::cout << "\nERROR: CreateRemoteThread filled thread variable with zero.\n";
				return -1;
			}

			// Block program execution to make sure that the dll as injected
			WaitForSingleObject(thread, INFINITE);
			GetExitCodeThread(thread, &exitCode);

			// Free the memory and clean up
			VirtualFreeEx(process, lpBaseAddress, 0, MEM_RELEASE);
			CloseHandle(thread);
			CloseHandle(process);
		}
	  // update the 'pe32' struct object to the next process
	} while (Process32Next(snapshot, &pe32));

	// Checks if the process exists
	if (process_found != pName)
	{
		std::cout << "ERROR: Process Not Found\n";
		return -1;
	}

	return 0;
}

/* Main function: takes the dll path and process name; call inject_dll() function */
int main()
{
	std::string dll_path;   // full DLL path
	std::wstring procName;  // process name
	std::ifstream dll_file; // the dll file: used to see if dll exists 

	// display the initial message
	display_ini_msg();

	// User types the process name
	std::cout << "Process Name: ";
	std::wcin >> procName;

	// User types the dll path
	std::cout << "\nDll Path: ";
	std::cin >> dll_path;

	// clean the screen
	system("cls");

	// checks if the dll path exists
	dll_file.open(dll_path.c_str());
	if (!dll_file)
	{
		std::cout << "\nERROR: Invalid DLL Path\n";
		return -1;
	}

	// Call the inject_dll() function and see if the function work's by checks for return value
	// if returns 0: done well! if returns -1: function don't work
	if (inject_dll(dll_path.c_str(), procName) == 0)
	{
		std::cout << "\n\n...DLL Injected...\n\n";
		return 0;
	}
	else { return -1; }
}


/* display_ini_msg(): display the initial mensage */
void display_ini_msg()
{
	std::cout << "\n*****************************************\n";
	std::cout << "*   x 8 6    D L L    I N J E C T O R   *\n";
	std::cout << "*****************************************\n";
	std::cout << "_______________skoomaster________________\n\n\n";
}