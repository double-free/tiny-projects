#include <cstdio>
#include <cstring> // for memcpy()
#include <fcntl.h> // for open()
#include <sys/mman.h>
#include <unistd.h> // for close()

#include "mmapper.h"

using namespace mem;

Writer::Writer(size_t size_lim, std::string file_path)
    : size_lim_(size_lim), file_path_(file_path), cur_pos_(0), pending_(0) {
  int fd = open(file_path_.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
  if (fd == -1) {
    printf("Open file %s failed.\n", file_path_.c_str());
    exit(-1);
  }

  // solve the bus error problem:
  // we should allocate space for the file first.
  lseek(fd, size_lim_ - 1, SEEK_SET);
  write(fd, "", 1);

  mem_file_ptr_ = mmap(0, size_lim_, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
  if (mem_file_ptr_ == MAP_FAILED) {
    printf("Map failed.\n");
    exit(-1);
  }

  spinlock_.clear(std::memory_order_release);
  close(fd);
}

Writer::~Writer() {
  if (munmap(mem_file_ptr_, size_lim_) == -1) {
    printf("Unmap failed.\n");
  }
  // resize the file to actual size
  truncate(file_path_.c_str(), cur_pos_);
  printf("Safely quit mmap\n");
  mem_file_ptr_ = nullptr;
}

void Writer::write_data(const char *data, size_t len) {
  /* 这两条语句中间可能被打断，并不是安全的
  size_t old_pos = cur_pos_.load();
  cur_pos_ += len;
  */
  while (spinlock_.test_and_set(std::memory_order_acquire)) {
    // acquire spinlock
    // atomically set flag to true and return previous value
  }
  size_t old_pos = cur_pos_;
  cur_pos_ += len;
  if (cur_pos_ > size_lim_) {
    remap(size_lim_ << 1);
  }
  spinlock_.clear(std::memory_order_release); // release spinlock

  pending_ += 1;
  std::memcpy((char *)mem_file_ptr_ + old_pos, data, len);
  pending_ -= 1;
}

void Writer::remap(size_t new_size) {
  // should be very time consuming, try to avoid

  // wait for all pending memcpy
  while (pending_ != 0) {
  }

  void *new_addr = mremap(mem_file_ptr_, size_lim_, new_size, MREMAP_MAYMOVE);
  if (new_addr == MAP_FAILED) {
    printf("Panic when try to remap...\n");
    return;
  }

  // extend file
  int fd = open(file_path_.c_str(), O_RDWR);
  if (fd == -1) {
    printf("Open file %s failed.\n", file_path_.c_str());
    exit(-1);
  }
  lseek(fd, new_size - 1, SEEK_SET);
  write(fd, "", 1);
  close(fd);

  if (new_addr != mem_file_ptr_) {
    printf("REMAP: map address changed from %p to %p...\n", mem_file_ptr_,
           new_addr);
    mem_file_ptr_ = new_addr;
  }
  size_lim_ = new_size;

  printf("REMAP: extend limit to %08x\n", new_size);
}
