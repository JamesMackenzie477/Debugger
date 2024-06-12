#include <iostream>
#include "Debugger.h"

using namespace std;

// defines the breakpoint callback
BOOL CALLBACK OnBreakpoint(LPVOID lpParameter)
{
	// notifies user
	cout << "Hit a breakpoint" << endl;
	// resumes execution
	return TRUE;
}

// the program entry point
int main(int argc, char *argv[])
{
	// finds the specified process
	if (DWORD dwProcessId = Process::FindProcess("notepad.exe"))
	{
		// attaches to the process
		if (Debugger* pDebugger = Debugger::Attach(dwProcessId))
		{
			// sets a breakpoint
			if (pDebugger->SetBreakpoint((LPVOID)0x00000000FF033ACC, OnBreakpoint))
			{
				// waits for the breakpoint to trigger
				while (true)
				{
					// waits for a breakpoint
					pDebugger->WaitForBreakpoint();
				}
			}
		}
	}
}