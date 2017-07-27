#pragma once
#include <cstddef>
#include <string>

#define MIN(A,B) ((A)<(B)?(A):(B))

class RingBuffer {
  // 1 个生产者，1 个消费者时不需要锁
public:
  RingBuffer();
  explicit RingBuffer(size_t size);
  ~RingBuffer();

  inline size_t dataLength() {
    /*
       _____out______in_____
      |_空闲_|__占用__|_空闲_|   occupied = in - out

       _____in______out_____
      |_占用_|__空闲__|_占用_|   occupied = in + size - out
      由于是无符号类型, in < out 时会自动 wrap-around
    */
    return in_ - out_;
  }

  void setSize(size_t size);

  void putData(char* data, size_t datalen);
  void getData(char* buf, size_t datalen);
  void startWrite(const std::string& path, size_t time_interval = 1);
private:
  char *buffer_;
  size_t size_; // 缓冲区大小
  size_t in_;
  size_t out_;

  inline bool is_power_of_2_(size_t x) {
    return x != 0 && (x & (x-1)) == 0;
  }
  void writeDataToFile(const std::string& path);
};
