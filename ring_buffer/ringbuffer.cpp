#include "ringbuffer.h"
#include <assert.h>

RingBuffer::RingBuffer(size_t size): size_(size) {
  assert(is_power_of_2_(size_));
  buffer_ = new char[size];
  in_ = 0;
  out_ = 0;
}

RingBuffer::~RingBuffer() {
  if (buffer_) {
    delete[] buffer_;
    buffer_ = nullptr;
  }
}

void RingBuffer::putData_(unsigned char* data, size_t datalen) {
  // in_ & (size_ - 1) 与
  // in_ % size_
  // 作用相同
  if (data == nullptr || buffer_ == nullptr) return;
  datalen = MIN(datalen, size_ - in_ + out_);
  // len 是考虑到尾部存不下，需要放一部分去首部的情况
  size_t len = min(datalen, size_ - (in_ & (size_ - 1)));
  memcpy(buffer_ + (in_ & (size_-1)), data, len);
  memcpy(buffer_, data + len, datalen - len);
  in_ += datalen;
}

void RingBuffer::putData(unsigned char* data, size_t datalen) {
  std::lock_guard<std::mutex> lg(mu_);
  
}

void RingBuffer::getData_(char* buf, size_t datalen) {
  if (buf == nullptr || buffer_ == nullptr) return;
  // 假设 in_ - out_ = 10，但是 datalen = 100，则只读出 10 个
  datalen = MIN(datalen, in_ - out_)
  size_t len = MIN(datalen, size_ - (out_ & (size_ - 1)));
  memcpy(buf, buffer_ + (out_ & (size - 1)), len);
  memcpy(buf + len, buffer_, datalen - len);
  out_ += datalen;
}
