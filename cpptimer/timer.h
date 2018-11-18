#include <chrono>
#include <iostream>

namespace utility {
// avoid to work around function with a *void* return value
// use constructor-destructor to calculate time
class TimePrinter {
private:
  std::chrono::steady_clock::time_point start_;

public:
  TimePrinter() : start_(std::chrono::steady_clock::now()) {}
  ~TimePrinter() {
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> time_span =
        std::chrono::duration_cast<std::chrono::duration<double>>(end - start_);
    std::cout << time_span.count() << " seconds elapsed" << std::endl;
  }
};

template <typename Func, typename... Args>
auto with_timer(Func f, Args &&... args)
    -> decltype(f(std::forward<Args>(args)...)) {
  TimePrinter p;
  return f(std::forward<Args>(args)...);
}
}
