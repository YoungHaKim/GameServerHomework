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

	//done: dbContext의 SQL을 실행하고 그 결과를 IO thread풀로 보내기		
//	if (!dbContext->SQLExecute())
//	{
//		dbContext->OnFail(); ///# 이건 I/O 풀에서 수행되어야겠제?

//		printf_s("Failed to execute DB job. \n");
//		return;
//	}

// 	if (FALSE == PostQueuedCompletionStatus(GIocpManager->GetCompletionPort(), dwTransferred, completionKey, (LPOVERLAPPED)dbContext))
// 	{
// 		printf_s("Error in posting DB job result to Queue: %d \n", GetLastError());
// 		return;
// 	}


	dbContext->mSuccess = dbContext->SQLExecute(); ///# 이렇게 결과만 넣어 I/O풀로 보내면 거기서 Success/Fail 처리.
	GIocpManager->PostDatabaseResult(dbContext); ///# 여기에 있는데 위에 왜 따로? 코드를 전체적으로 보는 습관을 가지도록.

}

