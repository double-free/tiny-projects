#pragma once
#include <stdint.h>
#include <vector>
#include <stdexcept>
#include <sstream>

template<typename T>
class CircularBuf {
public:
  explicit CircularBuf(size_t n) {
    buf_ = std::vector<T>(n);
    item_num_ = before_ = rear_ = 0;
  }

  inline bool empty() const {
    return item_num_ == 0;
  }

  inline bool full() const {
    return item_num_ == buf_.size();
  }

  inline size_t capacity() const {
    return buf_.size();
  }

  inline size_t size() const {
    return item_num_;
  }

  std::string str() const {
    auto elems = this->items();
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < elems.size(); i++) {
      oss << elems[i];
      if (i != elems.size()-1) {
        oss << " ";
      }
    }
    oss << "]";
    return oss.str();
  }

  inline std::vector<T> items() const {
    std::vector<T> ret;
    size_t count = item_num_;
    size_t curIdx = before_;
    while(count != 0) {
      curIdx = next(curIdx);
      ret.push_back(buf_[curIdx]);
      --count;
    }
    return ret;
  };

  // return true on success
  inline void push(const T& t) {
    if (item_num_ >= buf_.size()) {
      throw std::logic_error("size exceeded.");
    }
    rear_ = next(rear_);
    buf_[rear_] = t;
    item_num_ +=1;
  }

  inline T poll() {
    if (item_num_ == 0) {
      throw std::logic_error("container is empty.");
    }
    before_ = next(before_);
    item_num_ -= 1;
    return buf_[before_];
  }

  inline T peek() const {
    if (item_num_ == 0) {
      throw std::logic_error("container is empty.");
    }
    return buf_[next(before_)];
  }
private:
  size_t next(size_t cur) const {
    return cur+1 >= buf_.size()? 0:cur+1;
  }
  std::vector<T> buf_;
  size_t item_num_;
  size_t before_;  // pop element from before_+1
  size_t rear_; // insert element to rear
};
