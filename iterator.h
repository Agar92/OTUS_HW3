#pragma once

#include <iterator>
//#include <cstddef>

template <typename T>
class Iterator 
{
  using iterator_category = std::forward_iterator_tag;
  using difference_type   = std::ptrdiff_t;
  using value_type        = T;
  using pointer           = T*;
  using reference         = T&;
public:
  Iterator(pointer ptr) : m_ptr(ptr) {}

  reference operator*() const { return *m_ptr; }
  pointer operator->() { return m_ptr; }
  Iterator& operator++() { m_ptr++; return *this; }  
  Iterator operator++(int) { Iterator tmp = *this; ++(*this); return tmp; }
  friend bool operator== (const Iterator& a, const Iterator& b) { return a.m_ptr == b.m_ptr; };
  friend bool operator!= (const Iterator& a, const Iterator& b) { return a.m_ptr != b.m_ptr; };  
  
private:
  pointer m_ptr;
};

