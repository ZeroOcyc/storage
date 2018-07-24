
#include <limits.h>  
#include <stddef.h>  
 
template <typename T, size_t BlockSize = 4096>  
class MemoryPool  
{  
  public:  
    /* Member types */  
    typedef T               value_type;       // T 的 value 类型  
    typedef T*              pointer;          // T 的 指针类型  
    typedef T&              reference;        // T 的引用类型  
    typedef const T*        const_pointer;    // T 的 const 指针类型  
    typedef const T&        const_reference;  // T 的 const 引用类型  
    typedef size_t          size_type;        // size_t 类型  
    typedef ptrdiff_t       difference_type;  // 指针减法结果类型  
  
    template <typename U> struct rebind {  
      typedef MemoryPool<U> other;  
    };  
  
    /* Member functions */  
    /* 构造函数 */  
    MemoryPool() throw();  
    MemoryPool(const MemoryPool& memoryPool) throw();  
    template <class U> MemoryPool(const MemoryPool<U>& memoryPool) throw();  
  
    /* 析构函数 */  
    ~MemoryPool() throw();  
  
    /* 元素取址 */  
    pointer address(reference x) const throw();  
    const_pointer address(const_reference x) const throw();  
  
    // Can only allocate one object at a time. n and hint are ignored  
    // 分配和收回一个元素的内存空间  
    pointer allocate(size_type n = 1, const_pointer hint = 0);  
    void deallocate(pointer p, size_type n = 1);  
 
    // 可达到的最多元素数  
    size_type max_size() const throw();  
  
    // 基于内存池的元素构造和析构  
    void construct(pointer p, const_reference val);  
    void destroy(pointer p);  
  
    // 自带申请内存和释放内存的构造和析构  
    pointer newElement(const_reference val);  
    void deleteElement(pointer p);  
  
  private:  
    // union 结构体,用于存放元素或 next 指针  
    union Slot_ {  
      value_type element;  
      Slot_* next;  
    };  
  
    typedef char* data_pointer_;  // char* 指针，主要用于指向内存首地址  
    typedef Slot_ slot_type_;     // Slot_ 值类型  
    typedef Slot_* slot_pointer_; // Slot_* 指针类型  
  
    slot_pointer_ currentBlock_;  // 内存块链表的头指针  
    slot_pointer_ currentSlot_;   // 元素链表的头指针  
    slot_pointer_ lastSlot_;      // 可存放元素的最后指针  
    slot_pointer_ freeSlots_;     // 元素构造后释放掉的内存链表头指针  
  
    size_type padPointer(data_pointer_ p, size_type align) const throw();  // 计算对齐所需空间  
    void allocateBlock();  // 申请内存块放进内存池  
};  
 
// 计算对齐所需补的空间  
template <typename T, size_t BlockSize>  
inline typename MemoryPool<T, BlockSize>::size_type  
	MemoryPool<T, BlockSize>::padPointer(data_pointer_ p, size_type align)  
	const throw()  
{  
	size_t result = reinterpret_cast<size_t>(p);  
	return ((align - result) % align);  
}  
 
/* 构造函数，所有成员初始化 */  
template <typename T, size_t BlockSize>  
MemoryPool<T, BlockSize>::MemoryPool()  
	throw()  
{  
	currentBlock_ = 0;  
	currentSlot_ = 0;  
	lastSlot_ = 0;  
	freeSlots_ = 0;  
}  
 
/* 复制构造函数,调用 MemoryPool 初始化*/  
template <typename T, size_t BlockSize>  
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool& memoryPool)  
	throw()  
{  
	MemoryPool();  
}  
 
/* 复制构造函数,调用 MemoryPool 初始化*/  
template <typename T, size_t BlockSize>  
template<class U>  
MemoryPool<T, BlockSize>::MemoryPool(const MemoryPool<U>& memoryPool)  
	throw()  
{  
	MemoryPool();  
}  
 
/* 析构函数，把内存池中所有 block delete 掉 */  
template <typename T, size_t BlockSize>  
MemoryPool<T, BlockSize>::~MemoryPool()  
	throw()  
{  
	slot_pointer_ curr = currentBlock_;  
	while (curr != 0) {  
		slot_pointer_ prev = curr->next;  
		// 转化为 void 指针，是因为 void 类型不需要调用析构函数,只释放空间  
		operator delete(reinterpret_cast<void*>(curr));  
		curr = prev;  
	}  
}  
 
/* 返回地址 */  
template <typename T, size_t BlockSize>  
inline typename MemoryPool<T, BlockSize>::pointer  
	MemoryPool<T, BlockSize>::address(reference x)  
	const throw()  
{  
	return &x;  
}  
 
/* 返回地址的 const 重载*/  
template <typename T, size_t BlockSize>  
inline typename MemoryPool<T, BlockSize>::const_pointer  
	MemoryPool<T, BlockSize>::address(const_reference x)  
	const throw()  
{  
	return &x;  
}  
 
// 申请一块空闲的 block 放进内存池  
template <typename T, size_t BlockSize>  
void  MemoryPool<T, BlockSize>::allocateBlock()  
{  
	// Allocate space for the new block and store a pointer to the previous one  
	// operator new 申请对应大小内存，返回 void* 指针  
	data_pointer_ newBlock = reinterpret_cast<data_pointer_>  
		(operator new(BlockSize));  
	// 原来的 block 链头接到 newblock  
	reinterpret_cast<slot_pointer_>(newBlock)->next = currentBlock_;  
	// 新的 currentblock_  
	currentBlock_ = reinterpret_cast<slot_pointer_>(newBlock);  
	// Pad block body to staisfy the alignment requirements for elements  
	data_pointer_ body = newBlock + sizeof(slot_pointer_);  
	// 计算为了对齐应该空出多少位置  
	size_type bodyPadding = padPointer(body, sizeof(slot_type_));  
	// currentslot_ 为该 block 开始的地方加上 bodypadding 个 char* 空间  
	currentSlot_ = reinterpret_cast<slot_pointer_>(body + bodyPadding);  
	// 计算最后一个能放置 slot_type_ 的位置  
	lastSlot_ = reinterpret_cast<slot_pointer_> (newBlock + BlockSize - sizeof(slot_type_) + 1);  
}  
 
// 返回指向分配新元素所需内存的指针  
template <typename T, size_t BlockSize>  
inline typename MemoryPool<T, BlockSize>::pointer  
	MemoryPool<T, BlockSize>::allocate(size_type, const_pointer)  
{  
	// 如果 freeSlots_ 非空，就在 freeSlots_ 中取内存  
	if (freeSlots_ != 0) {  
		pointer result = reinterpret_cast<pointer>(freeSlots_);  
		// 更新 freeSlots_  
		freeSlots_ = freeSlots_->next;  
		return result;  
	}  
	else {  
		if (currentSlot_ >= lastSlot_)  
			// 之前申请的内存用完了，分配新的 block  
				allocateBlock();  
		// 从分配的 block 中划分出去  
		return reinterpret_cast<pointer>(currentSlot_++);  
	}  
}  
 
// 将元素内存归还给 free 内存链表  
template <typename T, size_t BlockSize>  
inline void  MemoryPool<T, BlockSize>::deallocate(pointer p, size_type)  
{  
	if (p != 0) {  
		// 转换成 slot_pointer_ 指针，next 指向 freeSlots_ 链表  
		reinterpret_cast<slot_pointer_>(p)->next = freeSlots_;  
		// 新的 freeSlots_ 头为 p  
		freeSlots_ = reinterpret_cast<slot_pointer_>(p);  
	}  
}  
 
// 计算可达到的最大元素上限数  
template <typename T, size_t BlockSize>  
inline typename MemoryPool<T, BlockSize>::size_type  
	MemoryPool<T, BlockSize>::max_size()  
	const throw()  
{  
	size_type maxBlocks = size_type(-1) / BlockSize;  
	return (BlockSize - sizeof(data_pointer_)) / sizeof(slot_type_) * maxBlocks;  
}  
 
// 在已分配内存上构造对象  
template <typename T, size_t BlockSize>  
inline void  
	MemoryPool<T, BlockSize>::construct(pointer p, const_reference val)  
{  
	// placement new 用法，在已有内存上构造对象，调用 T 的复制构造函数，  
	new (p) value_type (val);  
}  
 
// 销毁对象  
template <typename T, size_t BlockSize>  
inline void  
	MemoryPool<T, BlockSize>::destroy(pointer p)  
{  
	// placement new 中需要手动调用元素 T 的析构函数  
	p->~value_type();  
}  
 
// 创建新元素  
template <typename T, size_t BlockSize>  
inline typename MemoryPool<T, BlockSize>::pointer  
	MemoryPool<T, BlockSize>::newElement(const_reference val)  
{  
	// 申请内存  
	pointer result = allocate();  
	// 在内存上构造对象  
	construct(result, val);  
	return result;  
}  
 
// 删除元素  
template <typename T, size_t BlockSize>  
inline void  
	MemoryPool<T, BlockSize>::deleteElement(pointer p)  
{  
	if (p != 0) {  
		// placement new 中需要手动调用元素 T 的析构函数  
		p->~value_type();  
		// 归还内存  
		deallocate(p);  
	}  
}  
