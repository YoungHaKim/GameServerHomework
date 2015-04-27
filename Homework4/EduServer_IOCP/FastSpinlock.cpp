#include "stdafx.h"
#include "Exception.h"
#include "FastSpinlock.h"
#include "LockOrderChecker.h"
#include "ThreadLocal.h"

FastSpinlock::FastSpinlock(const int lockOrder) : mLockFlag(0), mLockOrder(lockOrder)
{
}


FastSpinlock::~FastSpinlock()
{
}


void FastSpinlock::EnterWriteLock()
{
	/// �� ���� �Ű� �Ƚᵵ �Ǵ� ���� �׳� �н�
	if ( mLockOrder != LO_DONT_CARE)
		LLockOrderChecker->Push(this);

	while (true)
	{
		/// �ٸ����� writelock Ǯ���ٶ����� ��ٸ���.
		while (mLockFlag & LF_WRITE_MASK)
			YieldProcessor();

		// add lf_write_flag, and make sure we are the only writer
		// if not, then wait until we are
		if ((InterlockedAdd(&mLockFlag, LF_WRITE_FLAG) & LF_WRITE_MASK) == LF_WRITE_FLAG)  
		{
			/// �ٸ����� readlock Ǯ���ٶ����� ��ٸ���.
			// spin until no readers left
			while (mLockFlag & LF_READ_MASK)
				YieldProcessor();

			return;
		}

		InterlockedAdd(&mLockFlag, -LF_WRITE_FLAG);
	}

}

void FastSpinlock::LeaveWriteLock()
{
	InterlockedAdd(&mLockFlag, -LF_WRITE_FLAG);

	/// �� ���� �Ű� �Ƚᵵ �Ǵ� ���� �׳� �н�
	if (mLockOrder != LO_DONT_CARE)
		LLockOrderChecker->Pop(this);
}

void FastSpinlock::EnterReadLock()
{
	if (mLockOrder != LO_DONT_CARE)
		LLockOrderChecker->Push(this);

	while (true)
	{
		// no other writers
		/// �ٸ����� writelock Ǯ���ٶ����� ��ٸ���.
		while (mLockFlag & LF_WRITE_MASK)
			YieldProcessor();

		// DONE
		//TODO: Readlock ���� ���� (mLockFlag�� ��� ó���ϸ� �Ǵ���?)
		// if ( readlock�� ������ )
		//return;
		// else
		// mLockFlag ����

		// readers don't care about other readers
		// but should not enter the lock of other writers are in

		// obtain read lock
		// add 1 to read lock flag
		// so last 20 bits should be nonzero
		// upper 12 bits should be zero (lockflag XOR upper 12 bits should be 
		// otherwise, someone has a write lock
		// so we fail to obtain the lock
		// if this value tested with logical and with write mask 
		
		if ( (InterlockedAdd(&mLockFlag, 1) & LF_WRITE_MASK) == 0 )  // no write lock
		{
			return;
		}
		else
		{
			InterlockedAdd(&mLockFlag, -1);
		}

		


		
	}
}

void FastSpinlock::LeaveReadLock()
{
	//DONE
	//TODO: mLockFlag ó�� 
	InterlockedAdd(&mLockFlag, -1);

	if (mLockOrder != LO_DONT_CARE)
		LLockOrderChecker->Pop(this);
}