#pragma once
#include "MemoryPool.h"
#include <list>
#include <vector>
#include <deque>
#include <set>
#include <hash_set>
#include <hash_map>
#include <map>
#include <queue>

template <class T>
class STLAllocator
{
public:
	STLAllocator() = default;

	typedef T value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	template <class U>
	STLAllocator(const STLAllocator<U>&)
	{}

	template <class U>
	struct rebind
	{
		typedef STLAllocator<U> other;
	};

	void construct(pointer p, const T& t)
	{
		new(p)T(t);
	}

	void destroy(pointer p)
	{
		p->~T();
	}

	T* allocate(size_t n)
	{
		return static_cast<T*>(GMemoryPool->Allocate(n));
		//DONE
		//TODO: 메모리풀에서 할당해서 리턴
		//return static_cast<T*>(malloc(n*sizeof(T)));
	}

	void deallocate(T* ptr, size_t n)
	{
		GMemoryPool->Deallocate(ptr, n);
		//DONE
		//TODO: 메모리풀에 반납
		//free(ptr);
	}
};


template <class T>
struct xvector
{
	typedef std::vector<T, STLAllocator<T>> type;
};

template <class T>
struct xdeque
{
	typedef std::deque<T, STLAllocator<T>> type;
	//DONE
	//TODO: STL 할당자를 사용하는 deque를 type으로 선언
	//typedef ... type;
};
/*
Arranges elements of a given type in a linear arrangement and, like a vector, enables fast random access to any element, and efficient insertion and deletion at the back of the container. However, unlike a vector, the deque class also supports efficient insertion and deletion at the front of the container.
template <
class Type,
class Allocator=allocator<Type>
>
class deque
Parameters
Type
The element data type to be stored in the deque.
Allocator
The type that represents the stored allocator object that encapsulates details about the deque's allocation and deallocation of memory. This argument is optional, and the default value is allocator<Type>.
*/

template <class T>
struct xlist
{
	//DONE
	//TODO: STL 할당자 사용
	typedef std::list<T, STLAllocator<T>> type;
};
/*
template <
class Type,
class Allocator=allocator<Type>
>
class list
*/

template <class K, class T, class C = std::less<K> >
struct xmap
{
	typedef std::map<K, T, C, STLAllocator<std::pair<K, T>> > type;
	//DONE
	//TODO: STL 할당자 사용하는 map을  type으로 선언
	//typedef ... type;
};
/*
Used for the storage and retrieval of data from a collection in which each element is a pair that has both a data value and a sort key. The value of the key is unique and is used to automatically sort the data.
The value of an element in a map can be changed directly. The key value is a constant and cannot be changed. Instead, key values associated with old elements must be deleted, and new key values must be inserted for new elements.
template <
class Key,
class Type,
class Traits = less<Key>,
class Allocator=allocator<pair <const Key, Type> >
> class map;
Parameters
Key
The key data type to be stored in the map.
Type
The element data type to be stored in the map.
Traits
The type that provides a function object that can compare two element values as sort keys to determine their relative order in the map. This argument is optional and the binary predicate less<Key> is the default value.
Allocator
The type that represents the stored allocator object that encapsulates details about the map's allocation and deallocation of memory. This argument is optional and the default value is allocator<pair <constKey, Type> >.
*/

template <class T, class C = std::less<T> >
struct xset
{
	typedef std::set<T, C, STLAllocator<T>> type;
	//DONE
	//TODO: STL 할당자 사용하는 set을  type으로 선언
	//typedef ... type;
};
/*
The STL container class set is used for the storage and retrieval of data from a collection in which the values of the elements contained are unique and serve as the key values according to which the data is automatically ordered. The value of an element in a set may not be changed directly. Instead, you must delete old values and insert elements with new values.
template <
class Key,
class Traits=less<Key>,
class Allocator=allocator<Key>
>
class set
*/

template <class K, class T, class C = std::hash_compare<K, std::less<K>> >
struct xhash_map
{
	typedef std::hash_map<K, T, C, STLAllocator<std::pair<K, T>> > type;
};

template <class T, class C = std::hash_compare<T, std::less<T>> >
struct xhash_set
{
	typedef std::hash_set<T, C, STLAllocator<T> > type;
};

template <class T, class C = std::less<std::vector<T>::value_type> >
struct xpriority_queue
{
	typedef std::priority_queue<T, xvector<T>, C > type;
	//DONE
	//TODO: STL 할당자 사용하는 priority_queue을  type으로 선언
	//typedef ... type;
};
/*
template <
class Type,
class Container=vector<Type>,
class Compare=less<typename Container::value_type>
>
class priority_queue
*/

typedef std::basic_string<wchar_t, std::char_traits<wchar_t>, STLAllocator<wchar_t>> xstring;

