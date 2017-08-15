#pragma once
#include <string>
#include <atomic>

namespace mem {
class Writer {
public:
  explicit Writer(size_t size_lim, const std::string file_path);
  ~Writer();
  void write_data(const char* data, size_t len);

private:
  size_t size_lim_;
  void* mem_file_ptr_;
  std::string file_path_;
  std::atomic<size_t> cur_pos_;

};
}
