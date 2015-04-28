#pragma once
#include "Exception.h"
#include "TypeTraits.h"
#include "XTL.h"
#include "ThreadLocal.h"

class GrandCentralExecuter
{
public:
	typedef std::function<void()> GCETask;

	GrandCentralExecuter(): mRemainTaskCount(0)
	{}

	void DoDispatch(const GCETask& task)
	{
		CRASH_ASSERT(LThreadType == THREAD_IO_WORKER); ///< 일단 IO thread 전용

		
		if (InterlockedIncrement64(&mRemainTaskCount) > 1)
		{
			// DONE?
			//TODO: 이미 누군가 작업중이면 어떻게?
			// the loop  below runs all tasks in the queue
			// so why not just push the task 
			mCentralTaskQueue.push(task);			
		}
		else
		{
			/// 처음 진입한 놈이 책임지고 다해주자 -.-;

			mCentralTaskQueue.push(task);
			
			while (true)
			{
				GCETask task;
				if (mCentralTaskQueue.try_pop(task))
				{
					// DONE
					//TODO: task를 수행하고 mRemainTaskCount를 하나 감소 
					// mRemainTaskCount가 0이면 break;
					task();

					//InterlockedDecrement64(&mRemainTaskCount);

					//if (mRemainTaskCount == 0) break;
					
					///# 이렇게 깔끔하게..
					if (0 == InterlockedDecrement64(&mRemainTaskCount))
						break;
				}
			}
		}

	}


private:
	typedef concurrency::concurrent_queue<GCETask, STLAllocator<GCETask>> CentralTaskQueue;
	CentralTaskQueue mCentralTaskQueue;
	int64_t mRemainTaskCount;
};

extern GrandCentralExecuter* GGrandCentralExecuter;



template <class T, class F, class... Args>
void GCEDispatch(T instance, F memfunc, Args&&... args)
{
	/// shared_ptr이 아닌 녀석은 받으면 안된다. 작업큐에 들어있는중에 없어질 수 있으니..
	static_assert(true == is_shared_ptr<T>::value, "T should be shared_ptr");

	
	///# 이렇게 간단하게 된다. bind가 곧 task(std::function이다)
	auto bind = std::bind(memfunc, instance, std::forward<Args>(args)...);
	GGrandCentralExecuter->DoDispatch(bind);

	/*GrandCentralExecuter::GCETask* task = 
		new GrandCentralExecuter::GCETask(
		std::bind(memfunc, instance.get(), std::forward<Args>(args)...)
		);

	
	GGrandCentralExecuter->DoDispatch(task);
	*/
	//TODO: intance의 memfunc를 std::bind로 묶어서 전달
	//auto myTask = std::bind(&memfunc, std::forward<Args>(args)...);

	/*GGrandCentralExecuter->DoDispatch(
		reinterpret_cast<std::function<void()>>(myTask)
		);*/	
}