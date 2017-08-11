#pragma once
#include <string>
#include <atomic>

namespace mem {
class Writer {
public:
  explicit Writer(int size_lim, const std::string file_path);
  ~Writer();
  void write_data(const char* data, int len);

private:
  const int size_lim_;
  void* mem_file_ptr_;
  std::string file_path_;
  std::atomic<int> cur_pos_;
};
}
