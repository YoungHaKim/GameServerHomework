#include "stdafx.h"
#include "ThreadLocal.h"
#include "Exception.h"
#include "Log.h"
#include <fstream>
#include <DbgHelp.h>
#include <TlHelp32.h>
#include <strsafe.h>
#include "StackWalker.h"

#define MAX_BUFF_SIZE 1024

void MakeDump(EXCEPTION_POINTERS* e)
{
	TCHAR tszFileName[MAX_BUFF_SIZE] = { 0 };
	SYSTEMTIME stTime = { 0 };
	GetSystemTime(&stTime);
	StringCbPrintf(tszFileName,
		_countof(tszFileName),
		_T("%s_%4d%02d%02d_%02d%02d%02d.dmp"),
		_T("EduServerDump"),
		stTime.wYear,
		stTime.wMonth,
		stTime.wDay,
		stTime.wHour,
		stTime.wMinute,
		stTime.wSecond);

	HANDLE hFile = CreateFile(tszFileName, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return;

	MINIDUMP_EXCEPTION_INFORMATION exceptionInfo;
	exceptionInfo.ThreadId = GetCurrentThreadId();
	exceptionInfo.ExceptionPointers = e;
	exceptionInfo.ClientPointers = FALSE;

	//done: MiniDumpWriteDump를 사용하여 hFile에 덤프 기록
	/*
	Writes user-mode minidump information to the specified file.

	BOOL WINAPI MiniDumpWriteDump(
	_In_ HANDLE                            hProcess,
	_In_ DWORD                             ProcessId,
	_In_ HANDLE                            hFile,
	_In_ MINIDUMP_TYPE                     DumpType,
	_In_ PMINIDUMP_EXCEPTION_INFORMATION   ExceptionParam,
	_In_ PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	_In_ PMINIDUMP_CALLBACK_INFORMATION    CallbackParam
	);
	*/
	MiniDumpWriteDump(
		GetCurrentProcess(),		//PROCESS_ALL_ACCESS obtained
		GetCurrentProcessId(),
		hFile,
		MiniDumpNormal,
		nullptr,
		nullptr,
		nullptr);

	
	if (hFile)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}
	
}


LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS* exceptionInfo)
{
	if ( IsDebuggerPresent() )
		return EXCEPTION_CONTINUE_SEARCH ;

	
	THREADENTRY32 te32;
	DWORD myThreadId = GetCurrentThreadId();
	DWORD myProcessId = GetCurrentProcessId();

	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);  //The process identifier of the process to be included in the snapshot. This parameter can be zero to indicate the current process.
	/*
	CreateToolhelp32Snapshot function
	Takes a snapshot of the specified processes, as well as the heaps, modules, and threads used by these processes.

	TH32CS_SNAPTHREAD
	0x00000004
	Includes all threads in the system in the snapshot. To enumerate the threads, see Thread32First.
	To identify the threads that belong to a specific process, compare its process identifier to the th32OwnerProcessID member of the THREADENTRY32 structure when enumerating the threads.
	*/


	if (hThreadSnap != INVALID_HANDLE_VALUE)
	{
		te32.dwSize = sizeof(THREADENTRY32);

		/*
		//Retrieves information about the first thread of any process encountered in a system snapshot.

		BOOL WINAPI Thread32First(		
		_In_    HANDLE          hSnapshot,		//A handle to the snapshot returned from a previous call to the CreateToolhelp32Snapshot function.
		_Inout_ LPTHREADENTRY32 lpte		//A pointer to a THREADENTRY32 structure.
		);
		*/

		if (Thread32First(hThreadSnap, &te32))		
		{
			do
			{
				//done: 내 프로세스 내의 스레드중 나 자신 스레드만 빼고 멈추게..

				// thread belongs to my process and is not the main thread
				if (te32.th32OwnerProcessID == myProcessId && te32.th32ThreadID != myThreadId)
				{
					SuspendThread(
						OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID)
						);
				}
				

			} while (Thread32Next(hThreadSnap, &te32));

		}

		CloseHandle(hThreadSnap);
	}
		
	
	std::ofstream historyOut("EduServer_exception.txt", std::ofstream::out);
	
	/// 콜히스토리 남기고
	historyOut << "========== WorkerThread Call History ==========" << std::endl << std::endl;
	for (int i = 0; i < MAX_WORKER_THREAD; ++i)
	{
		if (GThreadCallHistory[i])
		{
			GThreadCallHistory[i]->DumpOut(historyOut);
		}
	}

	/// 콜성능 남기고
	historyOut << "========== WorkerThread Call Performance ==========" << std::endl << std::endl;
	for (int i = 0; i < MAX_WORKER_THREAD; ++i)
	{
		if (GThreadCallElapsedRecord[i])
		{
			GThreadCallElapsedRecord[i]->DumpOut(historyOut);
		}
	}

	/// 콜스택도 남기고
	historyOut << "========== Exception Call Stack ==========" << std::endl << std::endl;

	//done: StackWalker를 사용하여 historyOut에 현재 스레드의 콜스택 정보 남기기
	StackWalker stackWalker;
	stackWalker.SetOutputStream(&historyOut);
	stackWalker.ShowCallstack();
	
	/// 이벤트 로그 남기고
	LoggerUtil::EventLogDumpOut(historyOut);

	historyOut.flush();
	historyOut.close();

	/// 마지막으로 dump file 남기자.
	MakeDump(exceptionInfo);


	ExitProcess(1);
	/// 여기서 쫑

	return EXCEPTION_EXECUTE_HANDLER;
	
}