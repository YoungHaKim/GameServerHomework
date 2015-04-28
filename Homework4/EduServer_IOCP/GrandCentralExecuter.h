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
		CRASH_ASSERT(LThreadType == THREAD_IO_WORKER); ///< �ϴ� IO thread ����

		
		if (InterlockedIncrement64(&mRemainTaskCount) > 1)
		{
			// DONE?
			//TODO: �̹� ������ �۾����̸� ���?
			// the loop  below runs all tasks in the queue
			// so why not just push the task 
			mCentralTaskQueue.push(task);			
		}
		else
		{
			/// ó�� ������ ���� å������ �������� -.-;

			mCentralTaskQueue.push(task);
			
			while (true)
			{
				GCETask task;
				if (mCentralTaskQueue.try_pop(task))
				{
					// DONE
					//TODO: task�� �����ϰ� mRemainTaskCount�� �ϳ� ���� 
					// mRemainTaskCount�� 0�̸� break;
					task();

					//InterlockedDecrement64(&mRemainTaskCount);

					//if (mRemainTaskCount == 0) break;
					
					///# �̷��� ����ϰ�..
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
	/// shared_ptr�� �ƴ� �༮�� ������ �ȵȴ�. �۾�ť�� ����ִ��߿� ������ �� ������..
	static_assert(true == is_shared_ptr<T>::value, "T should be shared_ptr");

	
	///# �̷��� �����ϰ� �ȴ�. bind�� �� task(std::function�̴�)
	auto bind = std::bind(memfunc, instance, std::forward<Args>(args)...);
	GGrandCentralExecuter->DoDispatch(bind);

	/*GrandCentralExecuter::GCETask* task = 
		new GrandCentralExecuter::GCETask(
		std::bind(memfunc, instance.get(), std::forward<Args>(args)...)
		);

	
	GGrandCentralExecuter->DoDispatch(task);
	*/
	//TODO: intance�� memfunc�� std::bind�� ��� ����
	//auto myTask = std::bind(&memfunc, std::forward<Args>(args)...);

	/*GGrandCentralExecuter->DoDispatch(
		reinterpret_cast<std::function<void()>>(myTask)
		);*/	
}