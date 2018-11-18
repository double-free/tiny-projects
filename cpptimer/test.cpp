#include <thread>
#include <cassert>
#include "timer.h"

bool test_return() {
	auto f = [](size_t n) -> int {
		std::this_thread::sleep_for(std::chrono::milliseconds(n));
		return 0;
	};
	return utility::with_timer(f, 5) == 0;
}

bool test_void() {
	auto f = [](size_t n) {
		std::this_thread::sleep_for(std::chrono::milliseconds(n));
	};
	utility::with_timer(f, 5);
	return true;
}

int main(int argc, char* argv[]) {
	assert(test_return());
	assert(test_void());
    return 0;
}
