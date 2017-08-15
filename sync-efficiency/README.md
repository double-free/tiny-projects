简介
---
在看 c++11 标准库的时候看到一句话(page 1012)：
>a mutex might be a relatively expensive operation in both necessary resources
and latency of the exclusive access. So, instead of using mutexes and lock,
it might be worth using atomics instead.

于是我好奇，到底慢了多少。就设计了这个试验。

实验
---
两个线程作自增操作 10 万次，对比无同步，用锁同步，用原子操作同步三种方式的效率。

硬件配置一致。均为 2 核 i5 处理器。

- **OS X系统**
```
unsafe add: result = 55562, time = 0.000818593 seconds
mutex add: result = 100000, time = 0.330456 seconds
atomic add: result = 100000, time = 0.0035597 seconds
spinlock add: result = 100000, time = 0.0101915 seconds
```

- **Ubuntu系统**
```
unsafe add: result = 61700, time = 0.000705618 seconds
mutex add: result = 100000, time = 0.0101063 seconds
atomic add: result = 100000, time = 0.00189584 seconds
spinlock add: result = 100000, time = 0.0032098 seconds
```

结论
---
都可以得出 `锁 >> 自旋锁 > 原子操作 > 无同步` 的结论。
但是 OS X 系统的锁操作明显慢于 Linux 系统。
