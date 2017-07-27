#include "ringbuffer.h"
#include <assert.h>
#include <cstring> // for memcpy

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

void RingBuffer::copy_in_(char* data, size_t datalen, size_t in, size_t out) {
  size_t len = MIN(datalen, size_ - (in & (size_ - 1)));
  memcpy(buffer_ + (in & (size_-1)), data, len);
  memcpy(buffer_, data + len, datalen - len);
}

void RingBuffer::putData(char* data, size_t datalen) {
  if (data == nullptr || buffer_ == nullptr) return;
  size_t in, out;
  {
    std::lock_guard<std::mutex> lg(mu_);
    datalen = MIN(datalen, size_ - in_ + out_);
    in = in_;
    out = out_;
  }
  copy_in_(data, datalen, in, out);
  {
    std::lock_guard<std::mutex> lg(mu_);
    in_ += datalen;
  }
}

void RingBuffer::copy_out_(char* buf, size_t datalen, size_t in, size_t out) {
  size_t len = MIN(datalen, size_ - (out & (size_ - 1)));
  memcpy(buf, buffer_ + (out & (size_ - 1)), len);
  memcpy(buf + len, buffer_, datalen - len);
}

void RingBuffer::getData(char* buf, size_t datalen) {
  if (buf == nullptr || buffer_ == nullptr) return;
  size_t in, out;
  {
    std::lock_guard<std::mutex> lg(mu_);
    // 假设 in_ - out_ = 10，但是 datalen = 100，则只读出 10 个
    datalen = MIN(datalen, in_ - out_);
    in = in_;
    out = out_;
  }
  copy_out_(buf, datalen, in, out);
  {
    std::lock_guard<std::mutex> lg(mu_);
    out_ += datalen;
  }
}
