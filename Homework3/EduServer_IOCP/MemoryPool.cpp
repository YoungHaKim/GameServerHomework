#include "stdafx.h"
#include "Exception.h"
#include "MemoryPool.h"

MemoryPool* GMemoryPool = nullptr;


SmallSizeMemoryPool::SmallSizeMemoryPool(DWORD allocSize) : mAllocSize(allocSize)
{
	printf_s("SmallSizeMemoryPool ctor called, allocSize=%d \n", allocSize);

	CRASH_ASSERT(allocSize > MEMORY_ALLOCATION_ALIGNMENT);
	InitializeSListHead(&mFreeList);
}

MemAllocInfo* SmallSizeMemoryPool::Pop()
{		
	// DONE: InterlockedPopEntrySList�� �̿��Ͽ� mFreeList���� pop���� �޸𸮸� ������ �� �ִ��� Ȯ��. 
	MemAllocInfo* mem = static_cast<MemAllocInfo*>(InterlockedPopEntrySList(&mFreeList)); 

	if (NULL == mem)
	{
		// �Ҵ� �Ұ����ϸ� ���� �Ҵ�.
		mem = reinterpret_cast<MemAllocInfo*>(_aligned_malloc(mAllocSize, MEMORY_ALLOCATION_ALIGNMENT));
	}
	else
	{
		CRASH_ASSERT(mem->mAllocSize == 0);
	}

	InterlockedIncrement(&mAllocCount);
	return mem;
}

void SmallSizeMemoryPool::Push(MemAllocInfo* ptr)
{
	//DONE: InterlockedPushEntrySList�� �̿��Ͽ� �޸�Ǯ�� (������ ����) �ݳ�.
	InterlockedPushEntrySList(&mFreeList, ptr);

	InterlockedDecrement(&mAllocCount);
}

/////////////////////////////////////////////////////////////////////

MemoryPool::MemoryPool()
{
	memset(mSmallSizeMemoryPoolTable, 0, sizeof(mSmallSizeMemoryPoolTable));

	int recent = 0;

	for (int i = 32; i < 1024; i+=32)
	{
		SmallSizeMemoryPool* pool = new SmallSizeMemoryPool(i);
		for (int j = recent+1; j <= i; ++j)
		{
			mSmallSizeMemoryPoolTable[j] = pool;	// �� ���� SmallSizeMemoryPool�� ���� �����͸� ��� �����ؼ� �ִ���???
		}
		recent = i;
	}

	for (int i = 1024; i < 2048; i += 128)
	{
		SmallSizeMemoryPool* pool = new SmallSizeMemoryPool(i);
		for (int j = recent + 1; j <= i; ++j)
		{
			mSmallSizeMemoryPoolTable[j] = pool;
		}
		recent = i;
	}

	//DONE
	//TODO: [2048, 4096] ���� ������ 256����Ʈ ������ SmallSizeMemoryPool�� �Ҵ��ϰ� 
	//TODO: mSmallSizeMemoryPoolTable�� O(1) access�� �����ϵ��� SmallSizeMemoryPool�� �ּ� ���

	for (int i = 2048; i <= 4096; i += 256)
	{
		SmallSizeMemoryPool* pool = new SmallSizeMemoryPool(i);
		for (int j = recent + 1; j <= i; ++j)
		{
			mSmallSizeMemoryPoolTable[j] = pool;
		}
		recent = i;
	}
	

}

void* MemoryPool::Allocate(int size)
{
	MemAllocInfo* header = nullptr;
	int realAllocSize = size + sizeof(MemAllocInfo);

	if (realAllocSize > MAX_ALLOC_SIZE)
	{
		header = reinterpret_cast<MemAllocInfo*>(_aligned_malloc(realAllocSize, MEMORY_ALLOCATION_ALIGNMENT));
	}
	else
	{
		//DONE
		//TODO: SmallSizeMemoryPool���� �Ҵ�
		//header = ...; 

		SmallSizeMemoryPool* pool = mSmallSizeMemoryPoolTable[realAllocSize];  // get corresponding pool ptr
		header = pool->Pop();
		
	}

	return AttachMemAllocInfo(header, realAllocSize);
}

void MemoryPool::Deallocate(void* ptr, long extraInfo)
{
	MemAllocInfo* header = DetachMemAllocInfo(ptr);  // shifts back the size of the header so we can access the header
	header->mExtraInfo = extraInfo; ///< �ֱ� �Ҵ翡 ���õ� ���� ��Ʈ
	
	long realAllocSize = InterlockedExchange(&header->mAllocSize, 0); ///< �ι� ���� üũ ����  The function returns the initial value of the Target parameter.
	
	CRASH_ASSERT(realAllocSize> 0);

	if (realAllocSize > MAX_ALLOC_SIZE)
	{
		_aligned_free(header);
	}
	else
	{
		//DONE
		//TODO: SmallSizeMemoryPool�� (������ ����) push..		
		SecureZeroMemory(header, header->mExtraInfo);
		mSmallSizeMemoryPoolTable[realAllocSize]->Push(header);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
InterlockedPopEntrySList function
Removes an item from the front of a singly linked list. Access to the list is synchronized on a multiprocessor system.
Syntax

C++

PSLIST_ENTRY WINAPI InterlockedPopEntrySList(
_Inout_  PSLIST_HEADER ListHead
);

Parameters

ListHead [in, out]
Pointer to an SLIST_HEADER structure that represents the head of a singly linked list.
Return value

The return value is a pointer to the item removed from the list. If the list is empty, the return value is NULL.
Remarks

All list items must be aligned on a MEMORY_ALLOCATION_ALIGNMENT boundary; otherwise, this function will behave unpredictably. See _aligned_malloc.

*/

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
InterlockedPushEntrySList function
Inserts an item at the front of a singly linked list. Access to the list is synchronized on a multiprocessor system.
Syntax

C++

PSLIST_ENTRY WINAPI InterlockedPushEntrySList(
_Inout_  PSLIST_HEADER ListHead,
_Inout_  PSLIST_ENTRY ListEntry
);

Parameters

ListHead [in, out]
Pointer to an SLIST_HEADER structure that represents the head of a singly linked list.
ListEntry [in, out]
Pointer to an SLIST_ENTRY structure that represents an item in a singly linked list.
Return value

The return value is the previous first item in the list. If the list was previously empty, the return value is NULL.
Remarks

All list items must be aligned on a MEMORY_ALLOCATION_ALIGNMENT boundary; otherwise, this function will behave unpredictably. See _aligned_malloc.
*/