#include <iostream>
#include <vector>
#include <map>

#include "vector.h"

using std::cout;
using std::endl;

template <class T, std::size_t growSize = 1024>
class MemoryPool
{ 
  class Buffer
  {
    static const std::size_t blockSize = sizeof(T);
    uint8_t data[blockSize * growSize];
  public:
    T *getBlock(std::size_t index)
    {
      return reinterpret_cast<T *>(&data[blockSize * index]);
    }
  };

  Buffer* buffer = nullptr;
  std::size_t bufferedBlocks = growSize;

public:

  MemoryPool() = default;
  MemoryPool(MemoryPool &&memoryPool) = delete;
  MemoryPool(const MemoryPool &memoryPool) = delete;
  MemoryPool operator =(MemoryPool &&memoryPool) = delete;
  MemoryPool operator =(const MemoryPool &memoryPool) = delete;
  
  ~MemoryPool()
  {
    if(buffer) delete buffer;
  }

  T* allocate(size_t n)
  {
    //initialization of the data buffer "buffer":
    if(bufferedBlocks >= growSize)
    {
      buffer = new Buffer;
      bufferedBlocks = 0;
    }
    //shift the pointer:
    bufferedBlocks += n;
    //return the pointer to n bytes: 
    return buffer->getBlock(bufferedBlocks-n);
  }
  //dummy realization:
  void deallocate(T *pointer){(void)pointer;}
};

template <class T, std::size_t growSize = 1024>
class Allocator
{
public:
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;
  typedef T *pointer;
  typedef const T *const_pointer;
  typedef T &reference;
  typedef const T &const_reference;
  typedef T value_type;
  
  template <class U>
  struct rebind
  {
    typedef Allocator<U, growSize> other;
  };
  Allocator(){pool=new MemoryPool<T,growSize>;}
  ~Allocator()
  {
    delete pool;
  }
  pointer allocate(size_type n, const void *hint = 0)
  {
    (void)hint;
    return pool->allocate(n);
  }
  
  void deallocate(pointer p, size_type n)
  {
    (void)n;
    pool->deallocate(p);
  }

  void construct(pointer p, const_reference val)
  {
    new (p) T(val);
  }
  
  void destroy(pointer p)
  {
    p->~T();
  }
private:
  MemoryPool<T, growSize>* pool;
};

int factorial(int x)
{
  if(x<2) return 1;
  int res=1;
  for(int i=1; i<=x; ++i) res*=i;
  return res;
}

int main()
{
  std::map<int,int> m1;
  for(int i=0; i<10; ++i) m1[i]=factorial(i);
//for(const auto [k,v] : m1) cout<<k<<" "<<v<<endl;
  
  //!!!WARNING!!! in <const int,int> const is necessary!!!
  std::map<int, int, std::less<int>, Allocator<std::pair<const int,int>, 32>> m2;
  for(int i=0; i<10; ++i) m2[i]=factorial(i);
  for(auto [k,v] : m2) cout<<k<<" "<<v<<endl;

  dev::vector<int> myvector1;
  for(int i=0; i<10; ++i) myvector1.push_back(i+1);
  for(const auto i : myvector1) cout<<i<<" ";
  cout<<endl;

  dev::vector<int, Allocator<int>> myvector2;
  for(int i=0; i<10; ++i) myvector2.push_back(i+1);
  for(const auto i : myvector2) cout<<i<<" ";
  cout<<endl;
  
  return 0;
}
