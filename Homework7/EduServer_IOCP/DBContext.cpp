#include "stdafx.h"
#include "ThreadLocal.h"
#include "FastSpinlock.h"
#include "ClientSession.h"
#include "DBContext.h"


DatabaseJobContext::DatabaseJobContext(ClientSession* owner) : mSessionObject(owner), mSuccess(false)
{
	mSessionObject->AddRef();
}

DatabaseJobContext::~DatabaseJobContext()
{
	mSessionObject->ReleaseRef();
}

bool DatabaseJobContext::SQLExecute()
{
	//done: 이 함수는 반드시 DB스레드풀에서 수행되어야 한다. 그렇지 않으면 CRASH시키기
	CRASH_ASSERT(LThreadType == THREAD_DB_WORKER); 
	CRASH_ASSERT(LWorkerThreadId < MAX_DB_THREAD);  // 음 윗줄이랑 결국 같은 얘기인듯

	return OnSQLExecute();
}

void DatabaseJobContext::OnResult()
{
	//done: 이 함수는 반드시 IO스레드풀에서 수행되어야 한다. 그렇지 않으면 CRASH시키기
	CRASH_ASSERT(LThreadType == THREAD_IO_WORKER);
	CRASH_ASSERT(LWorkerThreadId >= MAX_DB_THREAD);

	if (mSuccess)
		OnSuccess();
	else
		OnFail();
}



