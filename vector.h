#pragma once

#include "iterator.h"


namespace dev{

template <class T, class Allocator = std::allocator<T> >
class vector {
public:
  void push_back(const T& x)
  {
    if(_size == _capacity)
    {
      _capacity = _capacity * 2 + 1;
      T* newData = alloc.allocate(_capacity);
      if(_size) std::copy(data, data + _capacity, newData);
      std::swap(newData, data);
      if(_size) alloc.deallocate(newData, _capacity);
    }
    alloc.construct(data + _size, x);
    ++_size;
  }
  T& operator [](int index){return data[index];}
  size_t size() const {return _size;}

  Iterator<T> begin() { return Iterator<T>(&data[0]); }
  Iterator<T> end()   { return Iterator<T>(&data[_size]); }
private:
    std::size_t _size = 0;
    std::size_t _capacity = 0;
    T* data = nullptr;
    Allocator alloc;
};

}//namespace dev
