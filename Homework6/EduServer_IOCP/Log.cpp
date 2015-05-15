#include "stdafx.h"
#include "Log.h"

#include <iostream>

void ThreadCallHistory::DumpOut(std::ostream& ost)
{
	//done: 현재 스레드의 call history를 ost 스트림에 쓰기
	// 엑셀, matlab 분석 가능하도록 csv 형식으로 

	uint64_t count = mCounter < MAX_HISTORY ? mCounter : MAX_HISTORY;

	ost << std::endl << std::endl;
	ost << "SECTION_BEGIN,Recent Call History" << std::endl;
	ost << "ThreadID, FUNC_SIG" << std::endl;	

	for (uint64_t i = 1; i <= count; ++i)
	{
		ost << mThreadId << "," << mHistory[(mCounter - i) % MAX_HISTORY] << std::endl;
	}

	ost << "SECTION_END,Recent Call History" << std::endl;
	ost << std::endl << std::endl;
}
	

void ThreadCallElapsedRecord::DumpOut(std::ostream& ost)
{
	// 엑셀, matlab 분석 가능하도록 csv 형식으로 

	uint64_t count = mCounter < MAX_ELAPSED_RECORD ? mCounter : MAX_ELAPSED_RECORD;

	ost << std::endl << std::endl;
	ost << "SECTION_BEGIN,Recent Call Performance" << std::endl;
	ost << "ThreadID, FUNC, ELAPSED" << std::endl;

	for (uint64_t i = 1; i <= count; ++i)
	{
		ost << mThreadId << ","
			<< mElapsedFuncSig[(mCounter - i) % MAX_ELAPSED_RECORD] << ","
			<< mElapsedTime[(mCounter - i) % MAX_ELAPSED_RECORD] << std::endl;
	}
	ost << "SECTION_END,Recent Call Performance" << std::endl << std::endl;
	ost << std::endl << std::endl;
}


namespace LoggerUtil
{
	LogEvent gLogEvents[MAX_LOG_SIZE];
	__int64 gCurrentLogIndex = 0;
	LARGE_INTEGER gStartQPC;
	LARGE_INTEGER gFrequencyQPC;
	
	void InitializeTime()
	{
		QueryPerformanceFrequency(&gFrequencyQPC);
		QueryPerformanceCounter(&gStartQPC);
	}

	void EventLogDumpOut(std::ostream& ost)
	{
		//done: gLogEvents내용 ost 스트림에 쓰기
		// 엑셀, matlab 분석 가능하도록 csv 형식으로 
		uint64_t count = gCurrentLogIndex < MAX_LOG_SIZE ? gCurrentLogIndex : MAX_LOG_SIZE;
		
		ost << "Logs,*****" << std::endl;
		ost << "microstamp, threadID, info, logMsg" << std::endl;

		for (uint64_t i = 1; i <= count; ++i)
		{
			uint64_t index = (gCurrentLogIndex - 1) % MAX_LOG_SIZE;
			ost << gLogEvents[index].mMicrosecTimestamp << ","
				<< gLogEvents[index].mThreadId << ","
				<< gLogEvents[index].mAdditionalInfo << ","
				<< gLogEvents[index].mMessage << std::endl;
		}

	}
}
