/*
 * memorypool.h - Fixed-size block memory pool
 *
 * What it does:
 *   The pool requests a large region from the system during construction,
 *   divides it into equal-sized blocks, and tracks them in a free list.
 *   alloc() removes a block from the head of the list; dealloc() returns a
 *   block to the head. When all blocks are in use, another region is allocated.
 *
 * Why it can be fast:
 *   Each allocation and release only updates a pointer, instead of repeatedly
 *   invoking a general-purpose allocator.
 *
 * Design trade-offs:
 *   - Each pool serves blocks of one size. For different allocation sizes,
 *     consider std::pmr::unsynchronized_pool_resource.
 *   - This implementation is not thread-safe. Synchronize alloc()/dealloc()
 *     before sharing a pool between threads.
 *   - Double frees and pointers returned to the wrong pool are not detected
 *     and can corrupt the free list.
 *
 * This is an educational example, not a production allocator. Use a
 * production allocator or the standard library for real applications.
 */


#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include <cstddef>
#include <stdexcept>

#define DEFAULT_BLOCK_SIZE 128

class MemoryPool {
public:
    MemoryPool(const MemoryPool&)            = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;
    MemoryPool(MemoryPool&&)                 = delete;   // Moving is disabled too.
    MemoryPool& operator=(MemoryPool&&)      = delete;
    
    MemoryPool(std::size_t blockSize = DEFAULT_BLOCK_SIZE, std::size_t initialBlocks = 10);
    ~MemoryPool();

    // Allocate and release memory blocks.
    void* alloc();
    void dealloc(void* ptr);
private:
    // Links to the next free block and the next chunk.
    struct Block {
        Block* next;
        Block* nextChunk;
    };

    // Block size and number of blocks per chunk.
    std::size_t blockSize;
    std::size_t totalBlocks;
    // Head of the free list and the first allocated chunk.
    Block* freeList;
    void* poolMemory;

    // Add another chunk to the pool.
    void expandPool();
};


/************************************************************************************/

// Construct the pool with a block size and an initial block count.
MemoryPool::MemoryPool(std::size_t blockSize, std::size_t initialBlocks){
    // Ensure each block can store the free-list links.
    if (blockSize < sizeof(Block)) {
        blockSize = sizeof(Block);
    }
    // Align block sizes to 8 bytes.
    blockSize = (blockSize + 7) & ~static_cast<std::size_t>(7);

    if (initialBlocks == 0) {
        throw std::invalid_argument("MemoryPool: initialBlocks must be > 0");
    }
    this->blockSize = blockSize;
    this->totalBlocks = initialBlocks;
    this->freeList = nullptr;
    this->poolMemory = nullptr;
    // Allocate the initial chunk.
    expandPool();
}

MemoryPool::~MemoryPool() {
    while (poolMemory) {
        Block* currentChunk = static_cast<Block*>(poolMemory);
        poolMemory = currentChunk->nextChunk;
        operator delete(currentChunk);
    }
}

void MemoryPool::expandPool() {
    // Allocate one metadata block plus the requested usable blocks.
    void* newMemory = operator new(blockSize * (totalBlocks + 1));
    // Use the first block to store the link to the next chunk.
    Block* newBlock = static_cast<Block*>(newMemory);
    // Initialize the first block.
    newBlock->next = nullptr;
    newBlock->nextChunk = nullptr;
    newBlock->next = reinterpret_cast<Block*>(static_cast<char*>(newMemory) + blockSize);

    // Link the usable blocks into the free list.
    Block* currentBlock = newBlock->next;
    for (std::size_t i = 1; i < totalBlocks; ++i) {
        currentBlock->next = reinterpret_cast<Block*>(static_cast<char*>(newMemory) + (1 + i) * blockSize);
        currentBlock->nextChunk = nullptr;
        currentBlock = currentBlock->next;
    }
    currentBlock->next = nullptr; // Terminate the free list.
    
    // Add the new chunk to the head of the chunk list.
    Block* tempblock = static_cast<Block*>(poolMemory);
    poolMemory = newBlock;
    newBlock->nextChunk = tempblock;

    // Point the free-list head to the first usable block in the new chunk.
    freeList = newBlock->next;
}

void* MemoryPool::alloc() {
    if (!freeList) {
        // Add another chunk when no free blocks remain.
        expandPool();
    }
    // Remove and return the first free block.
    Block* block = freeList;
    freeList = freeList->next;
    return block;
}

void MemoryPool::dealloc(void* ptr) {
    if (!ptr) return; // Ignore null pointers.
    // Return the block to the head of the free list.
    Block* block = static_cast<Block*>(ptr);
    block->next = freeList;
    freeList = block;
}

#endif
