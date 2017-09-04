#include <string>
#include <sstream>
#include <chrono>
#include <cstdio>
#include <iomanip>  // for setfill, setw

namespace common {
  class Logger{
  public:
    static std::string time_in_hh_mm_ss_mmm() {
      using namespace std::chrono;
      // get current time
      auto now = system_clock::now();

      // get number of milliseconds for the current second
      // (remainder after division into seconds)
      auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

      // convert to std::time_t in order to convert to std::tm (broken time)
      auto timer = system_clock::to_time_t(now);

      // convert to broken time
      std::tm bt = *std::localtime(&timer);

      std::ostringstream oss;

      oss << std::put_time(&bt, "%T"); // HH:MM:SS
      oss << '.' << std::setfill('0') << std::setw(3) << ms.count();

      return oss.str();
    }

    template <typename... Args>
    static void t_out(const char* fmt, Args... args) {
      fprintf(stdout, "[%s] ", time_in_hh_mm_ss_mmm().c_str());
      fprintf(stdout, fmt, args...);
    }

    template <typename... Args>
    static void t_err(const char* fmt, Args... args) {
      fprintf(stderr, "[%s] ", time_in_hh_mm_ss_mmm().c_str());
      fprintf(stderr, fmt, args...);
    }

    template <typename... Args>
    static void t_critical(const char* fmt, Args... args) {
      fprintf(stderr, "[%s] CRITICAL: ", time_in_hh_mm_ss_mmm().c_str());
      fprintf(stderr, fmt, args...);
      exit(-1);
    }
  };
}
