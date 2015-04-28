#include "stdafx.h"
#include "Exception.h"
#include "ThreadLocal.h"
#include "FastSpinlock.h"
#include "LockOrderChecker.h"

LockOrderChecker::LockOrderChecker(int tid) : mWorkerThreadId(tid), mStackTopPos(0)
{
	memset(mLockStack, 0, sizeof(mLockStack));
}

void LockOrderChecker::Push(FastSpinlock* lock)
{
	CRASH_ASSERT(mStackTopPos < MAX_LOCK_DEPTH);

	if (mStackTopPos > 0)
	{
		//CRASH_ASSERT(
		//	mLockStack[mStackTopPos]->GetLockFlag() > lock->GetLockFlag()
		//	);
		///# �̷���.. �ε��� ����
		CRASH_ASSERT(mLockStack[mStackTopPos - 1]->mLockOrder < lock->mLockOrder);

		//DONE
		/// ���� ���� �ɷ� �ִ� ���¿� �����Ѱ��� �ݵ�� ���� ���� �켱������ ���ƾ� �Ѵ�.
		//TODO: �׷��� ���� ��� CRASH_ASSERT gogo
		
	}

	mLockStack[mStackTopPos++] = lock;
}

void LockOrderChecker::Pop(FastSpinlock* lock)
{

	/// �ּ��� ���� ���� �ִ� ���¿��� �� ���̰�
	CRASH_ASSERT(mStackTopPos > 0);

	//DONE
	//TODO: �翬�� �ֱٿ� push�ߴ� �༮�̶� ������ üũ.. Ʋ���� CRASH_ASSERT

	//CRASH_ASSERT(
	//	lock->GetLockFlag() == mLockStack[mStackTopPos]->GetLockFlag()
	//	);
	///# ���������� �ε��� ����
	CRASH_ASSERT(mLockStack[mStackTopPos - 1] == lock);

	mLockStack[--mStackTopPos] = nullptr;

}