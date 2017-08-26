参考
---
[中文资料](http://www.cnblogs.com/huxiao-tee/p/4660352.html)<br/>
[英文资料](https://www.safaribooksonline.com/library/view/linux-system-programming/0596009585/ch04s03.html)<br/>
[使用场景](https://stackoverflow.com/questions/258091/when-should-i-use-mmap-for-file-access)<br/>

介绍
---
除了标准的文件 IO，例如 open, read, write，内核还提供接口允许应用将文件 map 到内存。使得内存中的一个字节与文件中的一个字节一一对应。

- __优势__
  - 读写文件避免了 `read()` 和 `write()` 系统调用，也避免了数据的拷贝。
  - 除了潜在的页错误，读写 map 后的文件不引起系统调用或者上下文切换。就像访问内存一样简单。
  - 多个进程 map 同一个对象，可以共享数据。
  - 可以直接使用指针来跳转到文件某个位置，不必使用 `lseek()` 系统调用。
- __劣势__
  - 内存浪费。由于必须要使用整数页的内存。
  - 导致难以找到连续的内存区域
  - 创建和维护映射和相关的数据结构的额外开销。在大文件和频繁访问的文件中，这个开销相比 read write 的 copy 开销小。

![mmap 原理](http://upload-images.jianshu.io/upload_images/4482847-a04d010b9c8e2391.png?imageMogr2/auto-orient/strip%7CimageView2/2/w/1240)

动机
---
希望能实现文件快速安全的并行写入。

使用方法
---
函数原型为：
```
#include <sys/mman.h>

void * mmap (void *addr,
             size_t len,
             int prot,
             int flags,
             int fd,
             off_t offset);
```
- __addr__
这个参数是建议地址（hint），没有特别需求一般设为0。这个函数会返回一个实际 map 的地址。

- __len__
文件长度。

- __prot__
表明对这块内存的保护方式，不可与文件访问方式冲突。
`PROT_NONE`
无权限，基本没有用
`PROT_READ`
读权限
`PROT_WRITE`
写权限
`PROT_EXEC`
执行权限

- __flags__
描述了映射的类型。
`MAP_FIXED`
开启这个选项，则 addr 参数指定的地址是作为必须而不是建议。如果由于空间不足等问题无法映射则调用失败。不建议使用。
`MAP_PRIVATE`
表明这个映射不是共享的。文件使用 copy on write 机制映射，任何内存中的改动并不反映到文件之中。也不反映到其他映射了这个文件的进程之中。如果只需要读取某个文件而不改变文件内容，可以使用这种模式。
`MAP_SHARED`
和其他进程共享这个文件。往内存中写入相当于往文件中写入。会影响映射了这个文件的其他进程。与 `MAP_PRIVATE`冲突。

- __fd__
文件描述符。进行 map 之后，文件的引用计数会增加。因此，我们可以在 map 结束后关闭 fd，进程仍然可以访问它。当我们 unmap 或者结束进程，引用计数会减少。

- __offset__
文件偏移，从文件起始算起。

如果失败，mmap 函数将返回 `MAP_FAILED`。

页面对齐
---
内存拥有独立权限的最小单位就是页。因此，mmap 的最小单位也是页。`addr` 和 `offset` 参数都必须页对齐，`len` 会被 roundup。被 roundup 的多余的内存会以 `\0` 填充。对这一部分的写入操作不会影响文件。我们可以通过如下方式获取本机的页面大小：

```c
#include <unistd.h>

long page_size = sysconf(_SC_PAGESIZE);
```


遇到的问题
---

1. __写入时发生错误__
```
bus error(core dump)
```
stackoverflow 大佬的原话：

>You are creating a new zero sized file, you can't extend the file size with mmap. You'll get a bus error when you try to write outside the content of the file.

因此使用 `lseek` 先把文件扩展到需要的大小。
```c
  // solve the bus error problem:
  // we should allocate space for the file first.
  lseek(fd, size_lim_-1, SEEK_SET);
  write(fd,"",1);
```

2. __文件权限设置__
```c
int fd = open(file_path_.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
```
打开的时候忘了加 0644 设置权限。

3. __文件大小__

由于文件最初利用 lseek 扩张了一次，中间有大量的'\0'段。导致文件在验证中出错，而且打开缓慢。
```c
// resize the file to actual size
truncate(file_path_.c_str(), cur_pos_.load());
```
在析构函数中增加 truncate 解决。

4. __remap 过程中的 memcpy__

由于 remap 可能会切换地址，导致之前 map 的地址失效<br/>
而 memcpy 在设计中又是异步，所以会导致向失效的地址中写东西的情况发生，导致 segmentation fault。<br/>
解决方法：
暂时只能先用个引用计数，通过轮询确定为 0 再进行 remap <br/>
不可以使用 shared_ptr 实现引用计数，因为其引用计数的内存模型是 std::memory_order_relax，可能会乱序导致出错。

更新
---
为了处理文件超出初始分配的大小的问题，实现了 remap 的逻辑 <br/>
如果确保不需要 remap，整个过程可以只用一个 atomic 的 offset <br/>
关键的，实现了 spinlock 来保证各个线程的同步，保证在 remap 的过程中其他线程不再写入数据 <br/>
spinlock 可以采用 atomic_flag 来实现
