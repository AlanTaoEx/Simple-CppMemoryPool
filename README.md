---
title: C++ 实现的简单内存池
data: 2026-10-11
tags: [C++, 内存池, 内存管理, 性能优化]
---

# 利用 C++ 实现的简单内存池
### 简介
本文档介绍了一个使用 C++ 实现的简单内存池（Memory Pool）
- expandPool() 函数用于扩展内存池, 每个Chunk包含多个块，第一个块用于存储指向下一个Chunk的指针，剩余的块用于分配内存。当内存池中的块被分配完时，expandPool() 会创建一个新的Chunk，采用头插法将其加入到poolMemory中，并将新Chunk中的块加入到空闲链表中。这样可以在需要时动态扩展内存池的容量。
![内存池结构图](./img/image.png)
- allocate() 函数用于从内存池中分配内存块。它首先检查空闲链表是否为空，如果为空，则调用 expandPool() 扩展内存池。然后，它从空闲链表中取出一个块，将其从链表中移除，并返回该块的指针。这样可以快速地分配内存，而无需频繁调用系统的内存分配函数。
- deallocate() 函数用于将已分配的内存块归还给内存池。它将归还的块插入到空闲链表的头部，以便下次分配时可以快速获取。这样可以有效地管理内存，避免内存泄漏和碎片化问题。

> **Attention**: 该内存池实现是一个简单的示例，适用于学习和理解内存池的基本概念。不可用于生产环境，本案例不具有线程安全性，实际应用中需要根据具体需求进行改进和优化。
### 前置知识
- C++ 基础语法和面向对象编程
- 数据结构
- CMake
- debugging 和性能分析工具

### 运行环境

- 操作系统：Ubuntu 24.04.5 LTS（Linux 7.0.0-38-generic，x86_64）
- 编译器：GNU g++ 13.3.0
- CMake：3.28.3
- 构建工具：GNU Make 4.3
- 调试工具：GDB 15.1（可选，用于在 VS Code 中调试）
- 开发工具：VS Code（可选）

### 构建与运行

在项目根目录执行：

```bash
cmake -S . -B build
cmake --build build
```

运行测试程序：

```bash
./build/MemoryPoolTest
```

运行 CTest：

```bash
ctest --test-dir build --output-on-failure
```

### 内存池实现原理
- 几个重要的数据结构：
```cpp
struct Block {
    Block* next; // 指向下一个空闲块的指针
    Block* nextChunk; // 指向下一个Chunk的指针
};

    // 内存池的块大小和总块数
    std::size_t blockSize;
    std::size_t totalBlocks;

    // 空闲链表的头指针,用于给 allocate() 函数分配内存块
    Block* freeList;

    // 内存池的起始地址，用于内存管理和扩展内存池
    void* poolMemory;
```

- expandPool() 函数的实现：当内存池为空或者需要更多内存块时，调用该函数来扩展内存池。它会申请一片新的内存块，并将其划分为多个小块，然后将这些小块加入到空闲链表中，以便后续的分配操作可以使用。会多申请一个块用于存储指向下一个Chunk的指针，以便在需要时可以继续扩展内存池。
```cpp
void MemoryPool::expandPool() {
    // 先申请一片大小为 blockSize * (totalBlocks + 1) 的内存
    void* newMemory = operator new(blockSize * (totalBlocks + 1));
    // 将新申请的内存块加入空闲链表
    Block* newBlock = static_cast<Block*>(newMemory);
    // 初始化第一个块block
    newBlock->next = nullptr;
    newBlock->nextChunk = nullptr;
    newBlock->next = reinterpret_cast<Block*>(static_cast<char*>(newMemory) + blockSize);

    // 从第二个block开始，将新申请的内存块划分为多个小块，并加入空闲链表
    Block* currentBlock = newBlock->next;
    for (std::size_t i = 1; i < totalBlocks; ++i) {
        currentBlock->next = reinterpret_cast<Block*>(static_cast<char*>(newMemory) + (1 + i) * blockSize);
        currentBlock->nextChunk = nullptr;
        currentBlock = currentBlock->next;
    }
    currentBlock->next = nullptr; // 最后一个块的 next 指针为 nullptr
    
    // 使用头插法将新申请的内存块加入空闲链表
    Block* tempblock = static_cast<Block*>(poolMemory);
    poolMemory = newBlock;
    newBlock->nextChunk = tempblock;

    // 更新空闲链表的头指针
    freeList = newBlock->next;
}
```
- 这里顺带讲一讲四种类型转换函数
  - static_cast：用于大多数类型转换，包括基本类型、指针类型和引用类型。它在编译时进行类型检查，确保转换是安全的。
  - dynamic_cast：主要用于多态类型的安全向下转换。它在运行时检查对象的实际类型，如果转换不合法，则返回nullptr（对于指针）或抛出异常（对于引用）。
  - const_cast：用于去除对象的const或volatile属性。它可以将const对象转换为非const对象，但需要谨慎使用，以避免修改原本应该是只读的对象。
  - reinterpret_cast：用于低级别的类型转换，如将指针类型转换为整数类型，或者将一个指针类型转换为另一个不相关的指针类型。它不会进行任何类型检查，因此使用时需要非常小心，以避免未定义行为。

- expandPool() 流程如下:
1. 申请一片新的内存块，大小为 blockSize * (totalBlocks + 1)，其中多申请一个块用于存储指向下一个Chunk的指针。
2. 将新申请的内存块划分为多个小块，并将这些小块加入到空闲链表中。第一个块用于存储指向下一个Chunk的指针，剩余的块用于分配内存。
3. 使用头插法将新申请的内存块加入空闲链表。将新Chunk的nextChunk指针指向之前的poolMemory，然后更新poolMemory指针指向新Chunk。
4. 更新空闲链表的头指针freeList，指向新Chunk中的第一个可分配块。
![内存池扩展流程图](./img/image2.png)

> 恭喜，最为核心的内存池实现原理已经讲完了，剩下的就是一些流程了

### 内存池运行流程
- 构造函数`MemoryPool(std::size_t blockSize, std::size_t initialBlocks)`：初始化内存池，设置块大小和初始块数，并调用`expandPool()`函数分配初始内存块。
```cpp
MemoryPool::MemoryPool(std::size_t blockSize, std::size_t initialBlocks){
    // 确保块大小至少为一个指针的大小，以便存储空闲链表的指针
    if (blockSize < sizeof(Block)) {
        blockSize = sizeof(Block);
    }
    // 8字节对齐
    blockSize = (blockSize + 7) & ~static_cast<std::size_t>(7);

    // 验证initialBlocks是否为正数，如果不是，则抛出异常
    if (initialBlocks == 0) {
        throw std::invalid_argument("MemoryPool: initialBlocks must be > 0");
    }
    this->blockSize = blockSize;
    this->totalBlocks = initialBlocks;
    this->freeList = nullptr;
    this->poolMemory = nullptr;
    // 初始化内存池
    expandPool();
}
```
> 关于为何要进行8字节对齐的原因是为了提高内存访问的效率和性能。在大多数现代计算机体系结构中，内存访问是以字节为单位进行的，但处理器通常以更大的数据块（如4字节、8字节或16字节）为单位进行访问。对齐可以确保数据在内存中的起始地址是处理器所期望的边界，从而减少访问内存时的额外开销。但是实际测试中，8字节对齐的性能提升可能并不明显，甚至在某些情况下可能会略微降低性能。这是因为现代处理器和内存系统已经非常优化，能够处理非对齐的内存访问。

> "内存池不能拷贝"不是内存池的宿命，是"裸指针 + 手工释放"这个选择的必然结果。禁止所有拷贝操作 防止出现隐式拷贝的错误。

- 析构函数`~MemoryPool()`：释放内存池中所有分配的内存块，确保没有内存泄漏。
```cpp
MemoryPool::~MemoryPool() {
    // 释放所有分配的内存块
    Block* currentChunk = static_cast<Block*>(poolMemory);
    while (currentChunk) {
        Block* nextChunk = currentChunk->nextChunk;
        operator delete(currentChunk);
        currentChunk = nextChunk;
    }
}
```

- `allocate()` 函数：从内存池中分配一个块，如果空闲链表为空，则调用`expandPool()`扩展内存池。
```cpp
void* MemoryPool::allocate() {
    // 如果空闲链表为空，则扩展内存池
    if (!freeList) {
        expandPool();
    }
    
    // 从空闲链表中取出一个块
    Block* block = freeList;
    freeList = freeList->next;
    return block;
}
```

- `deallocate(void* ptr)` 函数：将已分配的块归还给内存池，将其插入到空闲链表的头部。
```cpp
void MemoryPool::deallocate(void* ptr) {
    if (!ptr) {
        return; // 如果指针为空，则直接返回
    }
    Block* block = static_cast<Block*>(ptr);
    block->next = freeList;
    freeList = block;
}
```

### 总结
本文档介绍了一个简单的 C++ 内存池实现，在开发的过程当中也遇到了很多的问题
1. v1.0 版本，仅仅实现了内存的分配和释放，没有好好考虑内存池的扩展问题，当扩展内存的时候，上一个Chunk块的地址丢失，导致内存泄漏。
2. v2.0 版本，考虑过很多方案来解决内存池扩展的问题:
    - 方案一：使用一个全局的链表来存储所有的Chunk块，这样可以在析构函数中释放所有的内存块，但是这样会增加额外的开销。会多一步malloc和free的开销，增加了内存池的复杂度。
    - 方案二：在每个Chunk块的末尾增加一个指针，指向下一个Chunk块，这样可以在析构函数中释放所有的内存块，但是这样会让Chunk块中存在blick大小不一样的问题，导致内存池的管理变得复杂。
    - 方案三：在每个Chunk块的第一个块中增加一个指针，指向下一个Chunk块，这样可以在析构函数中释放所有的内存块，并且不会增加额外的开销，只是会让Chunk块中存在一个额外的块，这个块不能被分配出去，增加了内存的开销，但是权衡利弊之后，还是选择了方案三，因为它的实现简单，并且不会增加性能额外的开销。
3. v3.0 版本，最终选择了方案三，但是在插入的时候选择了尾插法，在拓展内存的时候需要遍历整个链表，在内存池变很大时，会导致内存拓展的性能下降，最终选择了头插法，这样在拓展内存的时候，只需要修改一个指针即可，性能更高，并且实现起来更加的简单。
4. v3.1 版本，发现了bug，在构造函数中，如果initialBlocks为0，则会导致内存池无法正常工作，最终选择抛出异常，提示用户initialBlocks必须大于0 和 blocksize必须大于sizeof(Block)要不然无法存储next 和 nextChunk指针。
5. v3.2 版本，禁用了拷贝构造函数和拷贝赋值运算符，防止内存池被拷贝，导致内存泄漏和野指针的问题。

这就是一个简单的内存池实现的全过程，虽然这个内存池实现是一个简单的示例，但是它可以帮助我们理解内存池的基本概念和实现原理，并且可以作为一个基础，进一步扩展和优化，以满足不同的需求。