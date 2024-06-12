#include <Windows.h>
#include <TlHelp32.h>
#include <list>

// statically links the exploit tools library
#pragma comment(lib, "ExploitTools/ExploitTools.lib")
// includes the header used to interact with the exploit tools library
#include "ExploitTools/ExploitTools.h"

// safely edits a value
BOOL ProtectedEditValueEx(HANDLE hProcess, LPVOID lpAddress, LPVOID lpValue, SIZE_T nSize, LPVOID lpOld);

// defines the callback typedef
typedef BOOL(CALLBACK * Callback)(LPVOID lpParameter);

// defintion of a breakpoint object
struct Breakpoint
{
	// the address the breakpoint is set on
	DWORD64 lpAddress;
	// the callback to be called when the breakpoint is triggered
	Callback Handler;
	// the old value at the address
	WORD wOld;
	// constructs the structure
	Breakpoint();
	// constructs the structure
	Breakpoint(DWORD64 lpAddress, Callback Handler, WORD wOld);
};

// a class used for debugging without using windows debug api
class Debugger
{
private:
	// the pid of the attached process
	DWORD dwProcessId;
	// the handle of the process the debugger is attached to
	HANDLE hProcess;
	// is true if the process is 32 bit
	BOOL Wow64Process;
	// a list of breakpoints
	std::list<Breakpoint> Breakpoints;
	// creates a new debugger class instance
	Debugger(DWORD dwProcessId, HANDLE hProcess, BOOL Wow64Process);
	// checks if the given thread has triggered a breakpoint (64 bit processes only)
	VOID CheckBreakpoint64(DWORD dwThreadId);
	// checks if the given thread has triggered a breakpoint (32 bit processes only)
	VOID CheckBreakpoint32(DWORD dwThreadId);
	// checks if the given thread has triggered a breakpoint (universal wrapper)
	VOID CheckBreakpoint(DWORD dwThreadId);
public:
	// starts debugging the specified process
	static Debugger* Attach(DWORD dwProcessId);
	// sets a breakpoint at the specified address of the given program
	BOOL SetBreakpoint(LPVOID lpAddress, Callback Handler);
	// waits for a breakpoint event in the specified process
	VOID WaitForBreakpoint();
};