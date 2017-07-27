#include "ringbuffer.h"
#include <assert.h>
#include <cstring> // for memcpy
#include <fstream>
#include <chrono>
#include <thread>

RingBuffer::RingBuffer() {
  buffer_ = nullptr;
  in_ = 0;
  out_ = 0;
}

RingBuffer::RingBuffer(size_t size): size_(size) {
  assert(is_power_of_2_(size_));
  buffer_ = new char[size];
  in_ = 0;
  out_ = 0;
  printf("No lock ring buffer...\n" );
}

RingBuffer::~RingBuffer() {
  if (buffer_) {
    delete[] buffer_;
    buffer_ = nullptr;
  }
}

void RingBuffer::setSize(size_t size) {
  assert(is_power_of_2_(size));
  if (buffer_) {
    delete[] buffer_;
  }
  buffer_ = new char[size];
  size_ = size;
}

void RingBuffer::putData(char* data, size_t datalen) {
  if (data == nullptr || buffer_ == nullptr) return;
  datalen = MIN(datalen, size_ - in_ + out_);
  size_t len = MIN(datalen, size_ - (in_ & (size_ - 1)));
  memcpy(buffer_ + (in_ & (size_-1)), data, len);
  memcpy(buffer_, data + len, datalen - len);
  in_ += datalen;
  // printf("PUT: \n\tpos in increase to %lu, out = %lu, data length = %lu(of %lu)\n", in_, out_, dataLength(), size_);
}

void RingBuffer::getData(char* buf, size_t datalen) {
  if (buf == nullptr || buffer_ == nullptr) return;
  datalen = MIN(datalen, in_ - out_);
  size_t len = MIN(datalen, size_ - (out_ & (size_ - 1)));
  memcpy(buf, buffer_ + (out_ & (size_ - 1)), len);
  memcpy(buf + len, buffer_, datalen - len);
  out_ += datalen;
}

// for xtp market data
// 异步写文件
void RingBuffer::startWrite(const std::string& path, size_t time_interval) {
  auto f = [this](const std::string& path_, size_t time_interval_){
    printf("Worker Thread: %08x\n", std::this_thread::get_id());
    while (true) {
      this->writeDataToFile(path_);
      std::this_thread::sleep_for(std::chrono::seconds(time_interval_));
    }
  };
  std::thread t(f, std::ref(path), time_interval);
  t.detach();
}

void RingBuffer::writeDataToFile(const std::string& path) {
  size_t size_to_write = in_ - out_;
  std::ofstream fs(path, std::ios::out | std::ios::app);
  if (fs.is_open() == false || size_to_write == 0) {
    return;
  }
  size_t len = MIN(size_to_write, size_ - (out_ & (size_ - 1)));
  fs.write(buffer_ + (out_ & (size_ -1)), len);
  fs.write(buffer_, size_to_write - len);
  fs.close();
  out_ += size_to_write;
  // printf("GET: \n\tpos out increase to %lu, in = %lu, data length = %lu(of %lu)\n", out_, in_, dataLength(), size_);
}
