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
	//done: �� �Լ��� �ݵ�� DB������Ǯ���� ����Ǿ�� �Ѵ�. �׷��� ������ CRASH��Ű��
	CRASH_ASSERT(LThreadType == THREAD_DB_WORKER); 
	CRASH_ASSERT(LWorkerThreadId < MAX_DB_THREAD);  // �� �����̶� �ᱹ ���� ����ε�

	return OnSQLExecute();
}

void DatabaseJobContext::OnResult()
{
	//done: �� �Լ��� �ݵ�� IO������Ǯ���� ����Ǿ�� �Ѵ�. �׷��� ������ CRASH��Ű��
	CRASH_ASSERT(LThreadType == THREAD_IO_WORKER);
	CRASH_ASSERT(LWorkerThreadId >= MAX_DB_THREAD);

	if (mSuccess)
		OnSuccess();
	else
		OnFail();
}



