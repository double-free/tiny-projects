#pragma once
#include <cstddef>
#include <mutex>

#define MIN(A,B) ((A)<(B)?(A):(B))

class RingBuffer {
public:
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
    std::lock_guard<std::mutex> lg(mu_);
    return in_ - out_;
  }

  void putData(char* data, size_t datalen);
  void getData(char* buf, size_t datalen);

private:
  char *buffer_;
  size_t size_; // 缓冲区大小
  size_t in_;
  size_t out_;
  std::mutex mu_;

  inline bool is_power_of_2_(size_t x) {
    return x != 0 && (x & (x-1)) == 0;
  }


  /*
     _____out______in_____
    |_空闲_|__占用__|_空闲_|   free_size = out + size - in

     _____in______out_____
    |_占用_|__空闲__|_占用_|   free_size = out - in
  */
  // put data without lock
  void copy_in_(char* data, size_t datalen, size_t in, size_t out);
  // get data without lock
  void copy_out_(char* buf, size_t datalen, size_t in, size_t out);
};
