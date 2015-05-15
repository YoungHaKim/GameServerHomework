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
		/*todo: 스레드로컬에 함수 호출(__FUNCSIG__) 기록남기기*/ \
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
		//done: mElapsedFuncSig, mElapsedTime에 정보(funcsig, elapsed) 남기기
		// 이 객체는 thread 별로 존재하므로 lock 불 필요
				
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
		 * 10~16 ms 해상도로 체크하려면 GetTickCount 사용
		 * 1 us 해상도로 체크하려면  QueryPerformanceCounter 사용
		*/ 
		QueryPerformanceFrequency(&mFrequency);
		QueryPerformanceCounter(&mStartTick);
	}

	~ScopeElapsedCheck()
	{
		if (LThreadType != THREAD_MAIN)
		{
			//done: LThreadCallElapsedRecord에 함수 수행 시간 남기기
			//QueryPerformanceCOunter로 변경
			// 최종 계산 값은 실행에 소요된 microseconds

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
	
		//done: gLogEvents에 LogEvent정보 남기기

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

