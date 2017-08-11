#include <sys/mman.h>
#include <cstdio>
#include <fcntl.h> // for open()
#include <unistd.h> // for close()
#include <cstring> // for memcpy()

#include "mmapper.h"

using namespace mem;

Writer::Writer(int size_lim, std::string file_path):
  size_lim_(size_lim), file_path_(file_path), cur_pos_(0)
{
  int fd = open(file_path_.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
  if (fd == -1) {
    printf("Not a valid file: %s\n", file_path_.c_str());
    exit(-1);
  }

  // solve the bus error problem:
  // we should allocate space for the file first.
  lseek(fd, size_lim_-1, SEEK_SET);
  write(fd,"",1);

  mem_file_ptr_ = mmap(0, size_lim_, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
  if (mem_file_ptr_ == MAP_FAILED) {
    printf("Map failed.\n");
    exit(-1);
  }
  close(fd);
}

Writer::~Writer() {
  if (munmap(mem_file_ptr_, size_lim_) == -1) {
    printf("Unmap failed.\n");
  }
  mem_file_ptr_ = nullptr;
}

void Writer::write_data(const char* data, int len) {
  int old_pos = cur_pos_.load();
  cur_pos_ += len;
  std::memcpy((char *)mem_file_ptr_ + old_pos, data, len);
}
