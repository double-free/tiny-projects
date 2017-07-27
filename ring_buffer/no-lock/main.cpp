#include "ringbuffer.h"
#include <thread>
#include <random>
#include <chrono>
#include <cstddef>
// #include <sstream>

void putDataTo(RingBuffer* ringbuffer) {
  std::default_random_engine dre(0);
  std::uniform_int_distribution<int> uid(10, 100);
  const std::string s = "Hello, SB.";
  // std::ostringstream ss;
  for (int i=0; i<1000; i++) {
    // ss << "Hello, SB.";
    // std::string s = ss.str();
    ringbuffer->putData((char *)s.c_str(), s.size() + 1);
    // ss.clear();
    std::this_thread::sleep_for(std::chrono::milliseconds(uid(dre)));
  }
}

void getDataFrom(RingBuffer* ringbuffer) {
  std::default_random_engine dre(1);
  std::uniform_int_distribution<int> uid(50, 100);
  for (int i=0; i<1000; i++) {
    char buffer[20];
    size_t before = ringbuffer->dataLength();
    ringbuffer->getData(buffer, 11);
    printf("Get(%d): %s, Size %lu -> %lu\n", i, buffer, before, ringbuffer->dataLength());
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
