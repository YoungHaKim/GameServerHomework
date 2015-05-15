#pragma once

#include <iostream>
#include "Exception.h"
#include "ThreadLocal.h"

class ThreadCallHistory
{
public:
	ThreadCallHistory(int tid) : mThreadId(tid)
	{
		memset(mHistory, 0, sizeof(mHistory));
	}

	inline void Append(const char* funsig)
	{
		mHistory[mCounter++ % MAX_HISTORY] = funsig;
	}

	void DumpOut(std::ostream& ost = std::cout);

private:
	enum
	{
		MAX_HISTORY = 1024
	};

	uint64_t	mCounter = 0;
	int			mThreadId = -1;
	const char*	mHistory[MAX_HISTORY];
};


#define TRACE_THIS	\
	__if_exists (this)	\
	{	\
		LRecentThisPointer = (void*)this;	\
	}	\
	if (LThreadType != THREAD_MAIN)	\
	{	\
		/*todo: ��������ÿ� �Լ� ȣ��(__FUNCSIG__) ��ϳ����*/ \
	}	
	


class ThreadCallElapsedRecord
{
public:
	ThreadCallElapsedRecord(int tid) : mThreadId(tid)
	{
		memset(mElapsedFuncSig, 0, sizeof(mElapsedFuncSig));
		memset(mElapsedTime, 0, sizeof(mElapsedTime));
	}

	inline void Append(const char* funcsig, int64_t elapsedMicroseconds)
	{
		//done: mElapsedFuncSig, mElapsedTime�� ����(funcsig, elapsed) �����
		// �� ��ü�� thread ���� �����ϹǷ� lock �� �ʿ�
				
		mElapsedFuncSig[mCounter] = funcsig;
		mElapsedTime[mCounter] = elapsedMicroseconds;

		mCounter = ++mCounter % MAX_ELAPSED_RECORD;
	}

	void DumpOut(std::ostream& ost = std::cout);

private:
	enum
	{
		MAX_ELAPSED_RECORD = 512 * 64
	};

	uint64_t	mCounter = 0;
	int			mThreadId = -1;
	const char*	mElapsedFuncSig[MAX_ELAPSED_RECORD];
	int64_t		mElapsedTime[MAX_ELAPSED_RECORD];
};

class ScopeElapsedCheck
{
public:
	ScopeElapsedCheck(const char* funcsig) : mFuncSig(funcsig)
	{
		/* FYI
		 * 10~16 ms �ػ󵵷� üũ�Ϸ��� GetTickCount ���
		 * 1 us �ػ󵵷� üũ�Ϸ���  QueryPerformanceCounter ���
		*/ 
		QueryPerformanceFrequency(&mFrequency);
		QueryPerformanceCounter(&mStartTick);
	}

	~ScopeElapsedCheck()
	{
		if (LThreadType != THREAD_MAIN)
		{
			//done: LThreadCallElapsedRecord�� �Լ� ���� �ð� �����
			//QueryPerformanceCOunter�� ����
			// ���� ��� ���� ���࿡ �ҿ�� microseconds

			LARGE_INTEGER endTick;
			QueryPerformanceCounter(&endTick);

			LARGE_INTEGER elapsedMicroseconds;
			elapsedMicroseconds.QuadPart = (endTick.QuadPart - mStartTick.QuadPart) * 1000000;
			elapsedMicroseconds.QuadPart /= mFrequency.QuadPart;

			LThreadCallElapsedRecord->Append(mFuncSig, elapsedMicroseconds.QuadPart);
		}
	}

private:

	const char*		mFuncSig;
	LARGE_INTEGER	mStartTick;
	LARGE_INTEGER	mFrequency;
};

#define TRACE_PERF	\
	ScopeElapsedCheck __scope_elapsed_check__(__FUNCSIG__);


namespace LoggerUtil
{

	struct LogEvent
	{
		int mThreadId = -1;
		int	mAdditionalInfo = 0;
		const char* mMessage = nullptr; 
		uint64_t mMicrosecTimestamp = 0;
	};

	#define MAX_LOG_SIZE  16777216   ///< Must be a power of 2

	extern LogEvent gLogEvents[MAX_LOG_SIZE];
	extern __int64 gCurrentLogIndex;
	extern LARGE_INTEGER gStartQPC;
	extern LARGE_INTEGER gFrequencyQPC;

	void InitializeTime();

	inline void EventLog(const char* msg, int info)
	{
		__int64 index = _InterlockedIncrement64(&gCurrentLogIndex) - 1;
	
		//done: gLogEvents�� LogEvent���� �����

		index %= MAX_LOG_SIZE;
		
		gLogEvents[index].mMessage = msg;
		gLogEvents[index].mAdditionalInfo = info;
		gLogEvents[index].mThreadId = GetCurrentThreadId();

		LARGE_INTEGER endQPC, elapsedMicros;
		QueryPerformanceCounter(&endQPC);

		elapsedMicros.QuadPart = (endQPC.QuadPart - gStartQPC.QuadPart) * 1000000;

		gLogEvents[index].mMicrosecTimestamp = elapsedMicros.QuadPart /= gFrequencyQPC.QuadPart;

	}

	void EventLogDumpOut(std::ostream& ost = std::cout);
}

#define EVENT_LOG(x, info) LoggerUtil::EventLog(x, info)

