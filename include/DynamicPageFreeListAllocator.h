#pragma once

#include "NativeType.h"
#include "CoreAllocatorHelper.h"
#include "MemoryConstructor.h"
#include "LinkedMemoryUtility.h"

namespace CHRONE::MEMORY
{

template<
	class Allocator,
	Uint32 PageByteSize>
class DynamicPageFreeListAllocator
{
public:
	DynamicPageFreeListAllocator() = default;
	~DynamicPageFreeListAllocator();

	DynamicPageFreeListAllocator(const DynamicPageFreeListAllocator&) = delete;
	DynamicPageFreeListAllocator(DynamicPageFreeListAllocator&&) = delete;

	explicit DynamicPageFreeListAllocator(Allocator allocator);

	DynamicPageFreeListAllocator&	operator=(const DynamicPageFreeListAllocator&) = delete;
	DynamicPageFreeListAllocator&	operator=(DynamicPageFreeListAllocator&&) = delete;

	void*	MapMemory(Uint32 size);
	void	MapMemory(Uint32 size, Uint32 count, void** memory);

	void	UnmapMemory(void* memory);
	void	UnmapMemory(Uint32 count, void** memory);

	void	UnmapAllMemory();
	void	Reset();

private:

	struct FreeBlockHeader
	{
		FreeBlockHeader() = default;
		~FreeBlockHeader() = default;

		FreeBlockHeader(const FreeBlockHeader&) = delete;
		FreeBlockHeader(FreeBlockHeader&&) = delete;

		FreeBlockHeader(
			Uint32 size,
			FreeBlockHeader* nextFreeBlockPtr):
			size{ size },
			nextFreeBlockPtr{ nextFreeBlockPtr }
			{}

		FreeBlockHeader&	operator=(const FreeBlockHeader&) = delete;
		FreeBlockHeader&	operator=(FreeBlockHeader&&) = delete;


		Uint32	size{ 0u };
		FreeBlockHeader*	nextFreeBlockPtr{ nullptr };
	};

	struct BlockHeader
	{
		Uint32	size{ 0u };
	};

	struct PageHeader
	{
		PageHeader() = default;
		~PageHeader() = default;

		PageHeader(const PageHeader&) = delete;
		PageHeader(PageHeader&&) = delete;

		PageHeader(
			Uint32 size,
			Uint32 availableSize,
			FreeBlockHeader* freeBlocksBegin,
			PageHeader* nextPagePtr) :
			size{ size },
			availableSize{ availableSize },
			freeBlocksBegin{ freeBlocksBegin },
			nextPagePtr{ nextPagePtr }
		{}

		PageHeader&	operator=(const PageHeader&) = delete;
		PageHeader&	operator=(PageHeader&&) = delete;


		Uint32	size{ 0u };
		Uint32	availableSize{ 0u };
		FreeBlockHeader*	freeBlocksBegin{ nullptr };
		PageHeader*	nextPagePtr{ nullptr };
	};


	static constexpr auto FreeBlockHeaderSize{ static_cast<Uint32>(sizeof(FreeBlockHeader)) };
	static constexpr auto BlockHeaderSize{ static_cast<Uint32>(sizeof(BlockHeader)) };
	static constexpr auto PageHeaderSize{ static_cast<Uint32>(sizeof(PageHeader)) };
	static constexpr auto DefaultPageAvailableSize{ PageByteSize - PageHeaderSize - BlockHeaderSize };

	static_assert(
		static_cast<Uint32>(PageByteSize) > static_cast<Uint32>(PageHeaderSize), 
		"Page size must be greater than the page's header size");
	
	static PageHeader*	_FindAvailablePage(PageHeader* begin, Uint32 byteCount);
	static BlockHeader*	_FindAndSplitAvailableBlock(PageHeader* page, Uint32 byteCount);
	static void	_FindBLockInsert(FreeBlockHeader* begin, FreeBlockHeader const* blockHeader, FreeBlockHeader*& prev, FreeBlockHeader*& next);
	static void	_InsertAndCoalesceBlock(PageHeader* page, FreeBlockHeader* blockHeader);
	static Bool8	_LinkAndTryCoalesceBlock(FreeBlockHeader* prev, FreeBlockHeader* blockHeader);
	static void	_LinkPageHeader(PageHeader* dst, PageHeader* src);
	static void	_LinkFreeBlockHeader(FreeBlockHeader* dst, FreeBlockHeader* src);
	static PageHeader*	_Allocate(Allocator& allocator, Uint32 size);
	static void	_ConstructPage(PageHeader* pageHeader, Uint32 size);

	BlockHeader*	_FindPageAndSplitAvailableBlock(PageHeader* begin, Uint32 byteCount);
	PageHeader*	_FindMemoryPage(void* memory);

	void*	_MapMemory(Uint32 size);
	void	_MapMemory(Uint32 size, Uint32 count, void** memory);

	void	_UnmapMemory(void* memory);
	void	_UnmapMemory(Uint32 count, void** memory);

	void	_UnmapAllMemory();
	void	_Reset();

	Uint32	_pageCount{ 0u };
	PageHeader*	_pagesBegin{ nullptr };
	PageHeader*	_pagesEnd{ nullptr };

	Allocator	_allocator{};
};


template<
	class Allocator, 
	Uint32 PageByteSize>
inline 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::~DynamicPageFreeListAllocator()
{
	Reset();
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::DynamicPageFreeListAllocator(
	Allocator allocator)
{
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline void* 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::MapMemory(
	Uint32 size)
{
	if (!size) { return nullptr; }

	return _MapMemory(size);
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline void 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::MapMemory(
	Uint32 size, 
	Uint32 count, 
	void** memory)
{
	if (!size || !count || !memory) { return nullptr; }

	_MapMemory(size, count, memory);
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline void 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::UnmapMemory(
	void* memory)
{
	if (!memory) { return; }
	_UnmapMemory(memory);
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline void 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::UnmapMemory(
	Uint32 count, 
	void** memory)
{
	if (!count || !memory) { return; }
	_UnmapMemory(count, memory);
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline void 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::UnmapAllMemory()
{
	if (!_pagesBegin) { return; }
	_UnmapAllMemory();
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline void 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::Reset()
{
	if (!_pagesBegin) { return; }
	_UnmapAllMemory();
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline typename DynamicPageFreeListAllocator<Allocator, PageByteSize>::PageHeader* 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::_FindAvailablePage(
	PageHeader* begin, 
	Uint32 byteCount)
{
	PageHeader*	currentHeader{ begin };

	while (currentHeader)
	{
		if (currentHeader->availableSize >= byteCount)
		{
			return currentHeader;
		}

		currentHeader = currentHeader->nextPagePtr;
	}

	return nullptr;
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline typename DynamicPageFreeListAllocator<Allocator, PageByteSize>::BlockHeader* 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::_FindAndSplitAvailableBlock(
	PageHeader* page, 
	Uint32 byteCount)
{
	FreeBlockHeader*	prevHeader{ nullptr };
	FreeBlockHeader*	currentHeader{ page ? page->freeBlocksBegin : nullptr };

	byteCount = byteCount < FreeBlockHeaderSize ? FreeBlockHeaderSize : byteCount;

	while (currentHeader)
	{
		if (currentHeader->size >= byteCount)
		{
			const Uint32	formedBlockTotalSize{ 
				currentHeader->size - byteCount };

			FreeBlockHeader*	nextFreeBlockPtr{
				currentHeader->nextFreeBlockPtr };

			//We can split the block
			if (formedBlockTotalSize > FreeBlockHeaderSize)
			{
				FreeBlockHeader*	formedBlockHeader{
					reinterpret_cast<FreeBlockHeader*>(
						reinterpret_cast<Char*>(currentHeader) 
						+ BlockHeaderSize + byteCount) };

				formedBlockHeader->size = formedBlockTotalSize - BlockHeaderSize;
				_LinkFreeBlockHeader(formedBlockHeader, nextFreeBlockPtr);

				page->availableSize -= BlockHeaderSize;
				nextFreeBlockPtr = formedBlockHeader;
			}
			else
			{
				byteCount = currentHeader->size;
			}

			if (prevHeader)
			{
				_LinkFreeBlockHeader(prevHeader, nextFreeBlockPtr);
			}

			page->availableSize -= byteCount;
			currentHeader->size = byteCount;

			if (page->freeBlocksBegin == currentHeader)
			{
				page->freeBlocksBegin = nextFreeBlockPtr;
			}

			return reinterpret_cast<BlockHeader*>(currentHeader);
		}

		prevHeader = currentHeader;
		currentHeader = currentHeader->nextFreeBlockPtr;
	}

	return nullptr;
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline void
DynamicPageFreeListAllocator<Allocator, PageByteSize>::_FindBLockInsert(
	FreeBlockHeader* begin,
	FreeBlockHeader const* blockHeader,
	FreeBlockHeader*& prev,
	FreeBlockHeader*& next)
{
	prev = nullptr;
	next = begin;

	while (next && next < blockHeader)
	{
		prev = next;
		next = next->nextFreeBlockPtr;
	}
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline void 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::_InsertAndCoalesceBlock(
	PageHeader* page, 
	FreeBlockHeader* blockHeader)
{
	if (!page || !blockHeader) { return; }

	FreeBlockHeader*	freeBlockNewBegin{ page->freeBlocksBegin };
	Uint32	restoredMemorySize{ blockHeader->size };

	if (!freeBlockNewBegin)
	{
		freeBlockNewBegin = blockHeader;
		_LinkFreeBlockHeader(freeBlockNewBegin, nullptr);
		page->freeBlocksBegin = freeBlockNewBegin;
		return;
	}
	
	FreeBlockHeader*	insertPrevBlock{ nullptr };
	FreeBlockHeader*	insertNextBlock{ nullptr };

	_FindBLockInsert(freeBlockNewBegin, blockHeader, 
		insertPrevBlock, insertNextBlock);

	if (insertNextBlock == freeBlockNewBegin)
	{
		freeBlockNewBegin = blockHeader;
	}

	if (insertNextBlock)
	{
		if (_LinkAndTryCoalesceBlock(blockHeader, insertNextBlock))
		{
			restoredMemorySize += BlockHeaderSize;
		}
	}

	if (insertPrevBlock)
	{
		if (_LinkAndTryCoalesceBlock(insertPrevBlock, blockHeader))
		{
			restoredMemorySize += BlockHeaderSize;
		}
	}

	page->availableSize += restoredMemorySize;
	page->freeBlocksBegin = freeBlockNewBegin;

}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline Bool8
DynamicPageFreeListAllocator<Allocator, PageByteSize>::_LinkAndTryCoalesceBlock(
	FreeBlockHeader* prev, 
	FreeBlockHeader* blockHeader)
{
	_LinkFreeBlockHeader(prev, blockHeader);

	Char const*	const prevEnd{ 
		reinterpret_cast<Char const* const>(prev) + BlockHeaderSize + prev->size };

	Char const*	const blockHeaderRaw{ reinterpret_cast<Char const* const>(blockHeader) };

	if (prevEnd != blockHeaderRaw)
	{
		return false;
	}

	const Uint32 coalescedMemorySize{ BlockHeaderSize + blockHeader->size };
	prev->size += coalescedMemorySize;
	_LinkFreeBlockHeader(prev, blockHeader->nextFreeBlockPtr);

	return true;
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline void 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::_LinkPageHeader(
	PageHeader* dst, 
	PageHeader* src)
{
	LinkedMemoryUtility::WritePtrToMemory(
		static_cast<void*>(std::addressof(dst->nextPagePtr)), src);
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline void 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::_LinkFreeBlockHeader(
	FreeBlockHeader* dst, 
	FreeBlockHeader* src)
{
	LinkedMemoryUtility::WritePtrToMemory(
		static_cast<void*>(
			reinterpret_cast<BlockHeader*>(dst) + 1u), src);
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline typename DynamicPageFreeListAllocator<Allocator, PageByteSize>::PageHeader* 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::_Allocate(
	Allocator& allocator, 
	Uint32 size)
{
	//const Uint32	sizeDefaultAvailableSizeDiff{
	//	static_cast<Uint32>(size - DefaultPageAvailableSize)};
	size += PageHeaderSize + BlockHeaderSize;
	
	const Uint32	allocatedSize{ ((size - 1u) / PageByteSize + 1u) * PageByteSize };

	void*	memory{ 
		CoreAllocatorHelper<Allocator>::Allocate(allocator, allocatedSize) };

	PageHeader*	pageHeader{ reinterpret_cast<PageHeader*>(memory) };

	if (memory)
	{
		_ConstructPage(pageHeader, allocatedSize);
	}

	return pageHeader;
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline void 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::_ConstructPage(
	PageHeader* pageHeader, 
	Uint32 size)
{
	FreeBlockHeader*	pageFreeBlockBegin{ 
		reinterpret_cast<FreeBlockHeader*>(pageHeader + 1u) };

	const Uint32	availableSize{ size - PageHeaderSize - BlockHeaderSize };

	MemoryConstructor::Construct(pageHeader, size, availableSize,
		pageFreeBlockBegin, nullptr);

	MemoryConstructor::Construct(pageFreeBlockBegin, availableSize, nullptr);
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline typename DynamicPageFreeListAllocator<Allocator, PageByteSize>::BlockHeader* 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::_FindPageAndSplitAvailableBlock(
	PageHeader* begin, 
	Uint32 byteCount)
{
	PageHeader*	currentPage{ begin };
	BlockHeader*	blockHeader{ nullptr };

	while (currentPage && !blockHeader)
	{
		PageHeader*	availablePage{ _FindAvailablePage(currentPage, byteCount) };
		blockHeader = _FindAndSplitAvailableBlock(availablePage, byteCount);
		currentPage = availablePage ? availablePage->nextPagePtr : nullptr;
	}

	return blockHeader;
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline typename DynamicPageFreeListAllocator<Allocator, PageByteSize>::PageHeader* 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::_FindMemoryPage(
	void* memory)
{
	PageHeader*	currentPage{ _pagesBegin };
	char*	currentPageRawMemory{};

	while (currentPage)
	{
		currentPageRawMemory = reinterpret_cast<char*>(currentPage);
		
		if (currentPageRawMemory < memory &&
			(currentPageRawMemory + currentPage->size) > memory)
		{
			return currentPage;
		}

		currentPage = currentPage->nextPagePtr;
	}

	return nullptr;
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline void* 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::_MapMemory(
	Uint32 size)
{
	BlockHeader*	availableBlock{
		_FindPageAndSplitAvailableBlock(_pagesBegin, size) };

	if (!availableBlock)
	{
		PageHeader*	linkedPage{ _Allocate(_allocator, size) };

		if (!_pagesBegin)
		{
			_pagesBegin = linkedPage;
			_pagesEnd = _pagesBegin;
		}

		_LinkPageHeader(_pagesEnd, linkedPage);
		_pagesEnd = linkedPage;
		_LinkPageHeader(_pagesEnd, nullptr);

		availableBlock = _FindAndSplitAvailableBlock(linkedPage, size);
	}

	return (availableBlock + 1u);
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline void 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::_MapMemory(
	Uint32 size, 
	Uint32 count, 
	void** memory)
{
	for (Uint32 index{ 0u }; index < count; ++index)
	{
		memory[index] = _MapMemory(size);
	}
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline void 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::_UnmapMemory(
	void* memory)
{
	PageHeader*	page{ _FindMemoryPage(memory) };
	FreeBlockHeader*	block{ reinterpret_cast<FreeBlockHeader*>(
		reinterpret_cast<BlockHeader*>(memory) - 1u) };

	_InsertAndCoalesceBlock(page, block);
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline void 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::_UnmapMemory(
	Uint32 count, 
	void** memory)
{
	for (Uint32 index{ 0u }; index < count; ++index)
	{
		_UnmapMemory(memory[index]);
	}
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline void 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::_UnmapAllMemory()
{
	PageHeader*	currentPage{ _pagesBegin };

	while (currentPage)
	{
		_ConstructPage(currentPage, currentPage->size);
		currentPage = currentPage->nextPagePtr;
	}
}


template<
	class Allocator, 
	Uint32 PageByteSize>
inline void 
DynamicPageFreeListAllocator<Allocator, PageByteSize>::_Reset()
{
	PageHeader*	currentPage{ _pagesBegin };
	PageHeader*	nextPage{ nullptr };

	while (currentPage)
	{
		nextPage = currentPage->nextPagePtr;
		CoreAllocatorHelper<Allocator>::Deallocate(_allocator, currentPage);
		currentPage = nextPage;
	}

	_pagesBegin = nullptr;
	_pagesEnd = nullptr;
}

}