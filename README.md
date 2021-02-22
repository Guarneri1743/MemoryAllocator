# MemoryAllocator

A custom memory allocator based on explicit free list

There are several commonly used approaches to implement custom memory allocator, such as implicit free list, explicit free list, segregated free list and balanced binary search tree, each of them has its own pros and cons. here I use explicit free list to implement my custom memory allocator in purpose of studying and practicing the knowledge of memory management.

## Install

No binary libs required.

Option 1:

- Clone this [repo](https://github.com/Guarneri1743/MemoryAllocator.git) 
- Run 'setup.bat' to generate solution files

Option 2:

- Copy-paste the include folder to use it. Done.

## Usage

Simple Example:

	#include "MemoryAllocator.h"

	int main()
	{
		Allocator* allocator = new MemoryAllocator(16 MB);
		auto buffer1 = allocator->Allocate(64 KB);
		//...
		allocator->Free(buffer1);

		auto buffer2 = allocator->Allocate(1 MB);
		//...
		allocator->Free(buffer2);

		delete allocator;
	}

Specify PlacementPolicy:

	#include "MemoryAllocator.h"

	int main()
	{
		Allocator* allocator = new MemoryAllocator(16 MB, ExplicitFreeListAllocator::PlacementPolicy::kBestFit);
		auto buffer1 = allocator->Allocate(64 KB);
		//...
		allocator->Free(buffer1);

		auto buffer2 = allocator->Allocate(1 MB);
		//...
		allocator->Free(buffer2);

		delete allocator;
	}

Specify CoalescingPolicy: WIP

## Span & Boundary Tag
**Span** is used to represent an allocation or a free memory span in the memory layout. Extra space reserved for the boundary tags which indicate the metadata of memory span. Therefore the structure of span is as following.

- address(implicit)
- **header boundary tag(explicit)**
- **prev(explicit)**
- **next(explicit)**
- footer boundary tag(implicit)

**Boundary Tag:**

    struct BoundaryTag
    {
    	size_t size_and_flag; 
    };

size: 31 bits for payload size

flag: 1 bit (used/unused) for flag


**Span:**

    struct Span
    {
    	BoundaryTag size; 
    	Span* prev;
    	Span* next;
    };

## Free List

- A doubly linked list contains **Span** nodes. 
- The node of **Free List** is not guaranteed to be continueous in the memory layout.

## Allocation

#### First Fit: 

Iterate the free list until a free span fits the required allocation size

- Pros: fast
- Cons: more memory fragmentation



#### Next Fit: 

Iterate the free list starting from **last fit** until a free span fits

- Pros: relatively fast compared with **First Fit**
- Cons: more memory fragmentation

#### Best Fit: 

Iterate the free list, compare all of the free spans and choose the one with fewest size.

- Pros: less memory fragmentation
- Cons: relatively slow


#### Splitting:
A splitting is required when we find a free span but it still has extra bytes left. we need to split the free span into two spans and insert the last one into free list again. Some approaches can be used to optimize the splitting process, like deferred splitting or add a memory fragmentation tolerance.

## Free

#### LIFO/FIFO:
put the free span at the beginning or end of free list.

- Pros: fast
- Cons: more memory fragmentation

#### Address Ordered:
put the free span at address-ordered position of free list.

- Pros: less memory fragmentation
- Cons: relatively slow

#### Coalescing:

A coalescing is needed after a free span has been put into free list, in order to prevent the increasing of memory fragmentation.

## Future Work

- Use Red-Black tree or AVL tree to optimize the time complexity of allocation and free.
- Thread safe.