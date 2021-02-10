# MemoryAllocator

A custom memory allocator based on explicit free list

There are several commonly used approaches to implement custom memory allocator, such as implicit free list, explicit free list, segregated free list and balanced binary search tree, each of them has its own pros and cons. here I use explicit free list to implement my custom memory allocator in purpose of studying and practicing the knowledge of memory management.



## Span
**Span** is used to represent an allocation or a free memory span in the memory layout. Some essential information will be needed when we want to track the address, size and extra metadata of the allocation or free span. So the structure of a span is as following.

- address(implicit)
- size
- prev(explicit)
- next(explicit)

**Span structure:**

    struct Span
    {
    	size_type size; 
    	Span* prev;
    	Span* next;
    };

## Block
**Block** stands for each allocation from malloc(), a **Block** may contains many free **Span**s. A block list is maintained to track every allocation from malloc, so that we can free the allocations later. 

**Block structure:**

	struct Block
	{
		size_type size; //optional
		Block* prev;
		Block* next;
	};

## Memory Layout

Neither **Blocks** or **Spans** are guaranteed to be continueous in the memory layout.

## Allocation

#### First Fit: 

Iterate the free list util a free block fits the required allocation size

- Pros: fast
- Cons: more memory fragmentation



#### Next Fit: 

Iterate the free list starting from **last fit** util a free block fits

- Pros: relatively fast compared with **First Fit**
- Cons: more memory fragmentation

#### Best Fit: 

Iterate the free list, compare all of the free blocks and choose the one with fewest size.

- Pros: less memory fragmentation
- Cons: relatively slow


#### Splitting:
A splitting is required when we find a free block but it still has extra bytes left. we need to split the free block into two blocks and insert the last one into free list again. Some approaches can be used to optimize the splitting process, like deferred splitting or add a memory fragmentation tolerance.

## Free

#### LIFO/FIFO:
put the free block at the beginning or end of free list.

- Pros: fast
- Cons: more memory fragmentation

#### Sort by address:
put the free block at address-ordered position of free list.

- Pros: less memory fragmentation
- Cons: relatively slow

#### Merge:

Merging is needed when a free block has been put into free list, in order to prevent the increasing of free blocks.

## Future Work

Use Red-Black tree or AVL tree to optimize the time complexity of allocation and free.