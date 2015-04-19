#pragma once



/// Ŀ�����ϰ� ������ �Ҵ� �޴� �ֵ��� ���� �޸� ���� �ٿ��ֱ�
__declspec(align(MEMORY_ALLOCATION_ALIGNMENT))
struct MemAllocInfo : SLIST_ENTRY
{
	MemAllocInfo(int size) : mAllocSize(size), mExtraInfo(-1)
	{}
	
	long mAllocSize; ///< MemAllocInfo�� ���Ե� ũ��
	long mExtraInfo; ///< ��Ÿ �߰� ���� (��: Ÿ�� ���� ���� ��)

}; ///< total 16 ����Ʈ

inline void* AttachMemAllocInfo(MemAllocInfo* header, int size)
{
	//DONE: header�� MemAllocInfo�� ��ģ ������ ���� �ۿ��� ����� �޸� �ּҸ� void*�� ����... ���� ���Ǵ� �� �� DetachMemAllocInfo ����.

	header->mAllocSize = size;  // save actual size of memory used (includes header + data)
	++header; // increment ptr by amount of memAllocInfo size
	return reinterpret_cast<void*>(header);
}

inline MemAllocInfo* DetachMemAllocInfo(void* ptr)
{
	MemAllocInfo* header = reinterpret_cast<MemAllocInfo*>(ptr);
	--header;
	return header;
}

__declspec(align(MEMORY_ALLOCATION_ALIGNMENT))
class SmallSizeMemoryPool
{
public:
	SmallSizeMemoryPool(DWORD allocSize);

	MemAllocInfo* Pop();
	void Push(MemAllocInfo* ptr);
	

private:

	//An SLIST_HEADER structure is an opaque structure that serves as the header for a sequenced singly linked list. 
	SLIST_HEADER mFreeList; ///< �ݵ�� ù��° ��ġ  

	const DWORD mAllocSize;
	volatile long mAllocCount = 0;
};

class MemoryPool
{
public:
	MemoryPool();

	void* Allocate(int size);
	void Deallocate(void* ptr, long extraInfo);

private:
	enum Config
	{
		/// �Ժη� �ٲٸ� �ȵ�. ö���� ����� �ٲ� ��
		MAX_SMALL_POOL_COUNT = 1024/32 + 1024/128 + 2048/256, ///< ~1024���� 32����, ~2048���� 128����, ~4096���� 256����
		MAX_ALLOC_SIZE = 4096
	};

	/// ���ϴ� ũ���� �޸𸮸� ������ �ִ� Ǯ�� O(1) access�� ���� ���̺�
	SmallSizeMemoryPool* mSmallSizeMemoryPoolTable[MAX_ALLOC_SIZE+1];

};

extern MemoryPool* GMemoryPool;


/// ����� ��� �޾ƾ߸� xnew/xdelete ����� �� �ְ�...
struct PooledAllocatable {};


/*
when the ellipsis operator occurs to the right of a template or function call argument, it unpacks the parameter packs into separate arguments, like the args...
*/

template <class T, class... Args>
T* xnew(Args... arg)
{
	static_assert(true == std::is_convertible<T, PooledAllocatable>::value, "only allowed when PooledAllocatable");

	//DONE
	//TODO: T* obj = xnew<T>(...); ó�� ����� ���ֵ��� �޸�Ǯ���� �Ҵ��ϰ� ������ �ҷ��ְ� ����.
	
	// let's create an object of type T on the stack
	T someObject = T(arg...);   
	
	// then allocate from our custom memory pool
	int objectSize = sizeof(someObject);
	void* alloc = GMemoryPool->Allocate(objectSize);

	// then copy the new object to the pool memory
	memcpy_s(alloc, objectSize + sizeof(MemAllocInfo), &someObject, objectSize);
	
	return reinterpret_cast<T*>(alloc);
}

template <class T>
void xdelete(T* object)
{
	static_assert(true == std::is_convertible<T, PooledAllocatable>::value, "only allowed when PooledAllocatable");

	//DONE
	//TODO: object�� �Ҹ��� �ҷ��ְ� �޸�Ǯ�� �ݳ�.

	object->~T();  // call destructor

	GMemoryPool->Deallocate(object, sizeof(object));
	
}