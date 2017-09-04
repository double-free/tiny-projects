/************************************
 * Compare between mutex and atomic *
 ************************************/

#include <atomic>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <thread>

// 需要能整除
const int COUNTS = 100000;
const int THREAD_NUM = 2;

void unsafe_add() {
  size_t total = 0;
  std::thread threads[THREAD_NUM];
  int per_thread_count = COUNTS / THREAD_NUM;

  auto start = std::chrono::steady_clock::now();

  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i] = std::thread([&]() {
      for (int j = 0; j < per_thread_count; j++) {
        total += 1;
      }
    });
  }
  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i].join();
  }
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> diff = end - start;
  std::cout << "unsafe add: result = " << total << ", time = " << diff.count()
            << " seconds" << std::endl;
}

void safe_add() {
  size_t total = 0;
  std::thread threads[THREAD_NUM];
  int per_thread_count = COUNTS / THREAD_NUM;

  std::mutex mu;

  auto start = std::chrono::steady_clock::now();
  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i] = std::thread([&]() {
      for (int j = 0; j < per_thread_count; j++) {
        std::lock_guard<std::mutex> lg(mu);
        total += 1;
      }
    });
  }
  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i].join();
  }
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> diff = end - start;
  std::cout << "mutex add: result = " << total << ", time = " << diff.count()
            << " seconds" << std::endl;
}

void atomic_add() {
  std::atomic<size_t> total(0);
  std::thread threads[THREAD_NUM];
  int per_thread_count = COUNTS / THREAD_NUM;

  auto start = std::chrono::steady_clock::now();

  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i] = std::thread([&]() {
      for (int j = 0; j < per_thread_count; j++) {
        total.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }
  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i].join();
  }
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> diff = end - start;
  std::cout << "atomic add: result = " << total << ", time = " << diff.count()
            << " seconds" << std::endl;
}

void spinlock_add() {
  size_t total = 0;
  std::thread threads[THREAD_NUM];
  int per_thread_count = COUNTS / THREAD_NUM;
  std::atomic_flag spinlock = ATOMIC_FLAG_INIT;
  auto start = std::chrono::steady_clock::now();

  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i] = std::thread([&]() {
      for (int j = 0; j < per_thread_count; j++) {
        while (spinlock.test_and_set(std::memory_order_acquire)) {
        }
        total += 1;
        spinlock.clear(std::memory_order_release);
      }
    });
  }
  for (int i = 0; i < THREAD_NUM; i++) {
    threads[i].join();
  }
  auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double> diff = end - start;
  std::cout << "spinlock add: result = " << total << ", time = " << diff.count()
            << " seconds" << std::endl;
}

int main(int argc, const char *argv[]) {
  unsafe_add();
  safe_add();
  atomic_add();
  spinlock_add();
}
