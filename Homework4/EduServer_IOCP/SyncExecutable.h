#pragma once

#include "TypeTraits.h"
#include "FastSpinlock.h"
#include "Timer.h"


class SyncExecutable : public std::enable_shared_from_this<SyncExecutable>
{
public:
	SyncExecutable() : mLock(LO_BUSINESS_CLASS)
	{}
	virtual ~SyncExecutable() {}

	template <class R, class T, class... Args>
	R DoSync(R (T::*memfunc)(Args...), Args... args)
	{
		static_assert(true == std::is_convertible<T, SyncExecutable>::value, "T should be derived from SyncExecutable");

		//TODO: mLock으로 보호한 상태에서, memfunc를 실행하고 결과값 R을 리턴

		///# 템플릿 공부 좀 해야겠네? 아래처럼.. 보다시피 알고보면 초간단..
		FastSpinlockGuard lockGuard(mLock);
		return (static_cast<T*>(this)->*memfunc)(args...);
	
	}
	

	void EnterLock() { mLock.EnterWriteLock(); }
	void LeaveLock() { mLock.LeaveWriteLock(); }
	
 	template <class T>
	std::shared_ptr<T> GetSharedFromThis()
 	{
		static_assert(true == std::is_convertible<T, SyncExecutable>::value, "T should be derived from SyncExecutable");
		
		return std::static_pointer_cast<T>(shared_from_this());
		
		// DONE
		//TODO: this 포인터를 std::shared_ptr<T>형태로 반환.
		//(HINT: 이 클래스는 std::enable_shared_from_this에서 상속받았다. 그리고 static_pointer_cast 사용)

		//return std::shared_ptr<T>((Player*)this); ///< 이렇게 하면 안될걸???
 	}

private:

	FastSpinlock mLock;
};


template <class T, class F, class... Args>
void DoSyncAfter(uint32_t after, T instance, F memfunc, Args&&... args)
{
	static_assert(true == is_shared_ptr<T>::value, "T should be shared_ptr");
	static_assert(true == std::is_convertible<T, std::shared_ptr<SyncExecutable>>::value, "T should be shared_ptr SyncExecutable");

	//TODO: instance의 memfunc를 bind로 묶어서 LTimer->PushTimerJob() 수행

	///# 이렇게.. bind와 function객체에 대한 이해가 더 필요함.
	auto bind = std::bind(memfunc, instance, std::forward<Args>(args)...);
	LTimer->PushTimerJob(std::static_pointer_cast<SyncExecutable>(instance), bind, after);
	
}