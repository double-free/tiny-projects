
#include "circular_buf.h"
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>

std::mutex mu;
std::condition_variable cv;

void produce(CircularBuf<int> &buf, int producer_id, int product_id, size_t milli_sec = 10) {
  std::unique_lock<std::mutex> ul(mu);
  cv.wait(ul, [&]{
    printf("Producer %d: buffer(at address %p) %zu / %zu\n",
            producer_id, &buf, buf.size(), buf.capacity());
    return !buf.full();
  });
  // not full
  std::this_thread::sleep_for(std::chrono::milliseconds(milli_sec));
  buf.push(product_id);
  printf("Producer %d: produced %d...\n", producer_id, product_id);
  ul.unlock();
  cv.notify_one();
}

void consume(CircularBuf<int> &buf, int consumer_id, size_t milli_sec = 7) {
  std::unique_lock<std::mutex> ul(mu);
  cv.wait(ul, [&]{
    printf("Consumer %d: buffer(at address %p) %zu / %zu \n",
            consumer_id, &buf, buf.size(), buf.capacity());
    return !buf.empty();
  });
  // not empty
  std::this_thread::sleep_for(std::chrono::milliseconds(milli_sec));
  int num = buf.poll();
  printf("Consumer %d: consumed %d...\n", consumer_id, num);
  ul.unlock();
  cv.notify_one();
}

int main(int argc, char const *argv[]) {
  CircularBuf<int> buf(5);

  std::vector<std::thread> threads;
  int n = std::thread::hardware_concurrency();
  int product_num = 10;
  // half producer, half consumer
  for (int id = 0; id < n/2; id++) {
    threads.push_back(std::thread([&](int producer_id){
      // produce 10 products
      for (int i = 1; i <= product_num; i++) {
        produce(buf, producer_id, 500 + i);
        printf("++++ Producer %d: produced %dth product\n", producer_id, i);
        printf("Current buf: %s\n", buf.str().c_str());
      }
      printf("**** Producer %d quit\n", producer_id);
    }, id));

    threads.push_back(std::thread([&](int consumer_id){
      // consume 10 products
      for (int i = 1; i <= product_num; i++) {
        consume(buf, consumer_id);
        printf("---- Consumer %d: consumed %dth product\n", consumer_id, i);
        printf("Current buf: %s\n", buf.str().c_str());
      }
      printf("**** Consumer %d quit.\n", consumer_id);
    }, id));
  }

  printf("%lu threads created...\n", threads.size());

  for (auto& thread : threads) {
    thread.join();
  }

  return 0;
}

// compiled with: clang++ producer_consumer.cpp -lpthread -o app -std=c++11
