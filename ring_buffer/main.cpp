#include "ringbuffer.h"
#include <thread>
#include <random>
#include <chrono>

void putDataTo(RingBuffer* ringbuffer) {
  const std::string s = "Hello, SB.";
  std::default_random_engine dre(0);
  std::uniform_int_distribution<int> uid(10, 100);
  for (int i=0; i<1000; i++) {
    ringbuffer->putData((char *)s.c_str(), s.size());
    std::this_thread::sleep_for(std::chrono::milliseconds(uid(dre)));
  }
}

void getDataFrom(RingBuffer* ringbuffer) {
  std::default_random_engine dre(1);
  std::uniform_int_distribution<int> uid(10, 100);
  for (int i=0; i<1000; i++) {
    char buffer[20];
    ringbuffer->getData(buffer, 11);
    printf("Get: %s\n", buffer);
    std::this_thread::sleep_for(std::chrono::milliseconds(uid(dre)));
  }
}

int main() {
  RingBuffer ringbuf(4096);
  std::thread t1(putDataTo, &ringbuf);
  std::thread t2(getDataFrom, &ringbuf);
  t1.join();
  t2.join();
  return 0;
}
