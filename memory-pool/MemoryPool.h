#pragma once
#include <cstddef> // for size_t
#include <cstdint> // for uintptr_t
#include <utility> // for std::swap, std::forward

template <typename T, size_t BlockSize = 4096> class MemoryPool {
public:
  template <typename U> struct rebind { typedef MemoryPool<U> other; };
  /************
  * 构造与析构 *
  ************/
  MemoryPool() noexcept;                     // 构造函数
  MemoryPool(const MemoryPool &mp) noexcept; // 拷贝构造函数
  MemoryPool(MemoryPool &&mp) noexcept;      // 移动构造函数

  // 拷贝构造函数，但是是以另一个类型为模板
  template <class U> MemoryPool(const MemoryPool<U> &mp) noexcept;

  ~MemoryPool() noexcept;

  /*******
  * 赋值 *
  *******/
  //  使用 = delete 禁用拷贝赋值，只保留移动赋值
  MemoryPool &operator=(const MemoryPool &mp) = delete;
  MemoryPool &operator=(MemoryPool &&mp) noexcept;

  /*****************
  * 获取元素所在地址 *
  *****************/
  inline T *address(T &element) const noexcept { return &element; }
  inline const T *address(const T &element) const noexcept { return &element; }

  // 以下四个函数是 Allocator 必须实现的接口
  /**********************
  * 从内存池为对象分配内存 *
  **********************/
  inline T *allocate(size_t n = 1, const T *hint = nullptr) {
    if (freeSlots_ != nullptr) {
      // 存在 freeSlots
      T *result = reinterpret_cast<T *>(freeSlots_);
      freeSlots_ = freeSlots_->next;
      return result;
    } else {
      // freeSlots 还未初始化或者已经耗尽
      if (currentSlot_ >= lastSlot_) {
        allocateBlock();
      }
      return reinterpret_cast<T *>(currentSlot_++);
    }
  }

  /**********************
  * 从内存池为对象释放内存 *
  **********************/
  inline void deallocate(T *p, size_t = 1) {
    if (p != nullptr) {
      // 也是头插链表
      // 空闲链表 A->B->C，现在释放了 X 的空间，需要将 X 加入空闲链表
      // X->A->B->C
      reinterpret_cast<Slot_ *>(p)->next = freeSlots_;
      freeSlots_ = reinterpret_cast<Slot_ *>(p);
    }
  }

  /***************
  * 构造与析构对象 *
  ***************/
  template <typename U, typename... Args>
  inline void construct(U *p, Args &&... args) {
    new (p) U(std::forward<Args>(args)...);
  }
  template <typename U> inline void destroy(U *p) { p->~U(); }

  /**************
  * new与delete *
  **************/
  template <typename... Args> inline T *newElement(Args &&... args) {
    T *result = allocate();
    construct(result, std::forward<Args>(args)...);
    return result;
  }
  inline void deleteElement(T *p) {
    if (p != nullptr) {
      p->~T();
      deallocate(p);
    }
  }

  /********************
  * 最多能容纳多少个对象 *
  ********************/
  inline size_t max_size() const noexcept {
    // -1 表示当前能寻址的最大内存地址
    // -1 / BlockSize 就可以得出最大 Block 数
    size_t maxBlocks = -1 / BlockSize;
    // 每个 Block 能容纳的 T 的个数 乘 最大 Block 数
    return (BlockSize - sizeof(char *)) / sizeof(Slot_) * maxBlocks;
  }

private:
  // 为什么这里用 union?
  // 因为这里根本不是链表
  union Slot_ {
    T element;
    Slot_ *next;
  };

  Slot_ *currentBlock_;
  Slot_ *currentSlot_;
  Slot_ *lastSlot_;
  Slot_ *freeSlots_;  // 这是一个比较让人疑惑的名字，实际是指的 deallocate 之后空闲出来的 slots

  /*******
  * 对齐 *
  *******/
  inline size_t padPointer(char *p, size_t align) const noexcept {
    uintptr_t addr = reinterpret_cast<uintptr_t>(p);
    size_t padding = (align - addr) % align;
    // printf("addr = %08lx\n, align = %02lx, and padding = %02lx\n", addr, align, padding);
    return padding;
  }

  void allocateBlock();

  // static_assert: 编译期断言检查
  static_assert(BlockSize >= 2 * sizeof(Slot_), "BlockSize too small.");
};

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool() noexcept {
  currentBlock_ = nullptr;
  currentSlot_ = nullptr;
  lastSlot_ = nullptr;
  freeSlots_ = nullptr;
}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool &mp) noexcept
    : MemoryPool() {}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::MemoryPool(MemoryPool &&mp) noexcept {
  currentBlock_ = mp.currentBlock_;
  mp.currentBlock_ = nullptr;
  currentSlot_ = mp.currentSlot_;
  lastSlot_ = mp.lastSlot_;
  freeSlots_ = mp.freeSlots_;
}

template <typename T, size_t BlockSize>
template <typename U>
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool<U> &mp) noexcept
    : MemoryPool() {}

template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize>::~MemoryPool() noexcept {
  Slot_ *cur = currentBlock_;
  while (cur != nullptr) {
    Slot_ *nextBlock = cur->next;
    ::operator delete(reinterpret_cast<void *>(cur));
    cur = nextBlock;
  }
}

/**********
* 移动赋值 *
**********/
template <typename T, size_t BlockSize>
MemoryPool<T, BlockSize> &MemoryPool<T, BlockSize>::
operator=(MemoryPool &&mp) noexcept {
  if (this != &mp) {
    std::swap(currentSlot_, mp.currentSlot_);
    currentSlot_ = mp.currentSlot_;
    lastSlot_ = mp.lastSlot_;
    freeSlots_ = mp.freeSlots_;
  }
  return *this;
}

/*****************
* 为内存池分配内存 *
*****************/
template <typename T, size_t BlockSize>
void MemoryPool<T, BlockSize>::allocateBlock() {
  // 头插链表
  // A->B->C 插入 X
  // X->A->B->C
  char *newBlock = reinterpret_cast<char *>(::operator new(BlockSize));
  reinterpret_cast<Slot_ *>(newBlock)->next = currentBlock_;
  currentBlock_ = reinterpret_cast<Slot_ *>(newBlock);
  // 这里需要作一个内存对齐，因为第一个区域存放的是指向上一个 Block 的指针，而不是 Slot
  // 而之后的都是Slot
  char *body = newBlock + sizeof(Slot_ *);
  size_t bodyPadding = padPointer(body, alignof(Slot_));
  currentSlot_ = reinterpret_cast<Slot_ *>(body + bodyPadding);

  lastSlot_ =
      reinterpret_cast<Slot_ *>(newBlock + BlockSize - sizeof(Slot_) + 1);
}
