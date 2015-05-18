#include "stdafx.h"
#include "ThreadLocal.h"
#include "Exception.h"
#include "DBContext.h"
#include "DBThread.h"
#include "IocpManager.h"

DBThread::DBThread(HANDLE hThread, HANDLE hCompletionPort)
: mDbThreadHandle(hThread), mDbCompletionPort(hCompletionPort)
{}

DBThread::~DBThread()
{
	CloseHandle(mDbThreadHandle);
}

DWORD DBThread::Run()
{
	while (true)
	{
		DoDatabaseJob();
	}

	return 1;
}

void DBThread::DoDatabaseJob()
{
	DWORD dwTransferred = 0;
	LPOVERLAPPED overlapped = nullptr;
	ULONG_PTR completionKey = 0;

	int ret = GetQueuedCompletionStatus(mDbCompletionPort, &dwTransferred, (PULONG_PTR)&completionKey, &overlapped, INFINITE);

	if (CK_DB_REQUEST != completionKey)
	{
		CRASH_ASSERT(false);
		return;
	}

	
	DatabaseJobContext* dbContext = reinterpret_cast<DatabaseJobContext*>(overlapped);

	//done: dbContext�� SQL�� �����ϰ� �� ����� IO threadǮ�� ������		
//	if (!dbContext->SQLExecute())
//	{
//		dbContext->OnFail(); ///# �̰� I/O Ǯ���� ����Ǿ�߰���?

//		printf_s("Failed to execute DB job. \n");
//		return;
//	}

// 	if (FALSE == PostQueuedCompletionStatus(GIocpManager->GetCompletionPort(), dwTransferred, completionKey, (LPOVERLAPPED)dbContext))
// 	{
// 		printf_s("Error in posting DB job result to Queue: %d \n", GetLastError());
// 		return;
// 	}


	dbContext->mSuccess = dbContext->SQLExecute(); ///# �̷��� ����� �־� I/OǮ�� ������ �ű⼭ Success/Fail ó��.
	GIocpManager->PostDatabaseResult(dbContext); ///# ���⿡ �ִµ� ���� �� ����? �ڵ带 ��ü������ ���� ������ ��������.

}

