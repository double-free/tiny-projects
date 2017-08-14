#include <thread>
#include <atomic>
#include <string>
#include <vector>
#include <cstring>
#include <fstream>
#include "mmapper.h"

#define FILE_SIZE_LIM 1<<30 // 最多 1G

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    printf("Usage: %s <file_to_write>\n", argv[0]);
    exit(0);
  }

  mem::Writer writer(FILE_SIZE_LIM, std::string(argv[1]));
  std::atomic<size_t> total(0);

  int THREAD_NUM = std::thread::hardware_concurrency();

  // 读取要写入的单词
  std::ifstream infile("words.txt");
  std::vector<std::string> words;
  std::string line;
  while (getline(infile, line)) {
    words.push_back(std::move(line));
  }

  std::vector<std::thread> threads;
  for (int i=0; i<THREAD_NUM; i++) {
    threads.emplace_back([&](){
      std::string word_with_space = words[i % words.size()] + " ";
      const char *s = word_with_space.c_str();
      size_t len = word_with_space.size();
      for (int i=0; i< 1000; i++) {
        writer.write_data(s, len);
      }
      size_t byte_num = len*1000;
      total += byte_num;
      printf("Thread %08x put %lu bytes to file %s\n",
       std::this_thread::get_id(), byte_num, argv[1]);
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  printf("Write complete, %lu bytes in total\n", total.load());
  return 0;
}
