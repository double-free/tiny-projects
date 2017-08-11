#include <thread>
#include <atomic>
#include <string>
#include <vector>
#include <cstring>
#include "mmapper.h"

#define FILE_SIZE_LIM 1<<30 // 最多 1G

int main(int argc, const char* argv[]) {
  if (argc != 2) {
    printf("Usage: %s <file_to_write>\n", argv[0]);
    exit(0);
  }

  mem::Writer writer(FILE_SIZE_LIM, std::string(argv[1]));
  std::atomic<int> total(0);

  int THREAD_NUM = std::thread::hardware_concurrency();

  // 要写入的单词
  const char* words[] = {
    "hello ",
    "dog_and_cat ",
    "inu_to_neko ",
    "nb_and_sb "
  };

  std::vector<std::thread> threads;
  for (int i=0; i<THREAD_NUM; i++) {
    threads.emplace_back([&](){
      const char *s = words[i % (sizeof(words)/sizeof(words[0]))];
      int len = strlen(s);
      for (int i=0; i< 1000; i++) {
        writer.write_data(s, len);
      }
      int byte_num = len*1000;
      total += byte_num;
      printf("Thread %08x put %d bytes to file %s\n",
       std::this_thread::get_id(), byte_num, argv[1]);
    });
  }

  for (auto& t : threads) {
    t.join();
  }

  return 0;
}
