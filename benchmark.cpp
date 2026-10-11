#include<iostream>
#include "memorypool.h"

int main() {
    // Create a pool with 128-byte blocks and 20 initial blocks.
    MemoryPool pool(128, 20);
    // Allocate 1,000 blocks.
    void* allocatedBlocks[1000];
    for (int i = 0; i < 1000; ++i) {
        allocatedBlocks[i] = pool.alloc();
    }

    // Release the first 500 blocks.
    for (int i = 0; i < 500; ++i) {
        pool.dealloc(allocatedBlocks[i]);
    }

    // Allocate another 500 blocks.
    for (int i = 0; i < 500; ++i) {
        allocatedBlocks[i] = pool.alloc();
    }

    // Release all blocks.
    for (int i = 0; i < 1000; ++i) {
        pool.dealloc(allocatedBlocks[i]);
    }

    // Report that the memory-pool test completed.
    std::cout << "Memory pool test completed successfully." << std::endl;
    return 0;
}