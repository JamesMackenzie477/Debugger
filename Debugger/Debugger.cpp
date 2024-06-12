#include "Debugger.h"

// safely edits a value
BOOL ProtectedEditValueEx(HANDLE hProcess, LPVOID lpAddress, LPVOID lpValue, SIZE_T nSize, LPVOID lpOld)
{
	// stores the old memory protection
	DWORD lpflOldProtect;
	// changes the memory protection of the region
	if (VirtualProtectEx(hProcess, lpAddress, nSize, PAGE_READWRITE, &lpflOldProtect))
		// saves the old instruction at the address
		if (ReadProcessMemory(hProcess, lpAddress, lpOld, nSize, NULL))
			// writes the new instruction to the address
			if (WriteProcessMemory(hProcess, lpAddress, lpValue, nSize, NULL))
				// restores the memory protection
				if (VirtualProtectEx(hProcess, lpAddress, nSize, lpflOldProtect, &lpflOldProtect))
					// function succeeded
					return TRUE;
	// function failed
	return FALSE;
}

// constructs a breakpoint structure
Breakpoint::Breakpoint(DWORD64 lpAddress, Callback Handler, WORD wOld)
	: lpAddress(lpAddress), Handler(Handler), wOld(wOld)
{
}

// creates a new debugger class instance
Debugger::Debugger(DWORD dwProcessId, HANDLE hProcess, BOOL Wow64Process)
	: dwProcessId(dwProcessId), hProcess(hProcess), Wow64Process(Wow64Process)
{

}

// starts debugging the specified process
Debugger* Debugger::Attach(DWORD dwProcessId)
{
	// opens a handle to the target process
	if (HANDLE hProcess = Process::GetProcessHandle(dwProcessId, PROCESS_ALL_ACCESS))
	{
		// stores the process type
		BOOL Wow64Process;
		// checks the process type
		if (Process::IsProcess32(hProcess, &Wow64Process))
		{
			// creates and returns a new class instance
			return new Debugger(dwProcessId, hProcess, Wow64Process);
		}
	}
	// function failed
	return NULL;
}

// sets a breakpoint at the specified address of the given program
BOOL Debugger::SetBreakpoint(LPVOID lpAddress, Callback Handler)
{
	// stores the old instruction
	WORD wOld;
	// stores the new instruction
	WORD wNew = 0xFEEB;
	// writes the new instruction to the address
	if (ProtectedEditValueEx(hProcess, lpAddress, &wNew, sizeof(WORD), &wOld))
	{
		// adds the breakpoint to the breakpoint linked list
		Breakpoints.push_front(Breakpoint((DWORD64)lpAddress, Handler, wOld));
		// function succeeded
		return TRUE;
	}
	// function failed
	return FALSE;
}

// waits for a breakpoint event in the specified process
VOID Debugger::WaitForBreakpoint()
{
	// takes a snapshot of all current system threads
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwProcessId);
	// checks the handle
	if (snapshot != INVALID_HANDLE_VALUE)
	{
		// stores information about our thread
		THREADENTRY32 threadInfo;
		// initalizes our threadInfo structure
		threadInfo.dwSize = sizeof(THREADENTRY32);
		// gets the first thread
		if (Thread32First(snapshot, &threadInfo))
		{
			// iterates through all threads
			do
			{
				// checks the process id
				if (threadInfo.th32OwnerProcessID == dwProcessId)
				{
					// checks if a breakpoint has been triggered on the thread
					CheckBreakpoint(threadInfo.th32ThreadID);
				}
			} while (Thread32Next(snapshot, &threadInfo));
		}
	}
}

// checks if the given thread has triggered a breakpoint (64 bit processes only)
VOID Debugger::CheckBreakpoint64(DWORD dwThreadId)
{
	// opens a handle to the thread
	if (HANDLE hThread = Thread::GetThreadHandle(dwThreadId, THREAD_ALL_ACCESS))
	{
		// stores our thread context
		CONTEXT Context;
		// we want the full context
		Context.ContextFlags = CONTEXT_FULL;
		// gets the thread context
		if (GetThreadContext(hThread, &Context))
		{
			// iterates through the process breakpoints
			for (std::list<Breakpoint>::iterator iterator = Breakpoints.begin(); iterator != Breakpoints.end(); ++iterator)
			{
				// checks if the instruction address is that of a breakpoint
				if (iterator->lpAddress == Context.Rip)
				{
					// calls the handler
					if (iterator->Handler(NULL))
					{
						// resumes execution
						//if (ProtectedEditValueEx(hProcess, (LPVOID)iterator->lpAddress, &iterator->wOld, sizeof(WORD), NULL))
						//{

						//}
					}
				}
			}
		}
	}
}

// checks if the given thread has triggered a breakpoint (32 bit processes only)
VOID Debugger::CheckBreakpoint32(DWORD dwThreadId)
{
	// opens a handle to the thread
	if (HANDLE hThread = Thread::GetThreadHandle(dwThreadId, THREAD_ALL_ACCESS))
	{
		// stores our thread context
		WOW64_CONTEXT Context;
		// we want the full context
		Context.ContextFlags = WOW64_CONTEXT_FULL;
		// gets the thread context
		if (Wow64GetThreadContext(hThread, &Context))
		{
			// iterates through the process breakpoints
			for (std::list<Breakpoint>::iterator iterator = Breakpoints.begin(); iterator != Breakpoints.end(); ++iterator)
			{
				// checks if the instruction address is that of a breakpoint
				if (iterator->lpAddress == Context.Eip)
				{
					// calls the handler
					if (iterator->Handler(NULL))
					{
						// resumes execution
						//if (ProtectedEditValueEx(hProcess, (LPVOID)iterator->lpAddress, &iterator->wOld, sizeof(WORD), NULL))
						//{

						//}
					}
				}
			}
		}
	}
}

// checks if the given thread has triggered a breakpoint (universal wrapper)
VOID Debugger::CheckBreakpoint(DWORD dwThreadId)
{
	// checks if the process is 32 bit
	if (Wow64Process)
		// returns the result
		return CheckBreakpoint32(dwThreadId);
	// returns the result
	return CheckBreakpoint64(dwThreadId);
}

// suspends the process
BOOL SuspendProcess()
{

}