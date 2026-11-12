#ifndef RINGBUFFER_H
#define RINGBUFFER_H
#include <cstddef>

class RingBuffer {
public:
  RingBuffer(size_t size, int fillWith) {
    arr_ = new int[size];
    for (int i = 0; i < size; i++) {
      arr_[i] = fillWith;
    }
    size_ = size;
    index_ = 0;
  }

  void push(int val_) {
    index_ = (index_ + 1) % size_;

    arr_[index_] = val_;
  }

  int getAvg() {
    int sum{};

    for (int i = 0; i < size_; i++) {
      sum += arr_[i];
    }

    return sum / size_;
  }

private:
  int *arr_;
  size_t size_;
  size_t index_;
};

#endif
