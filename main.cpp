#include <iostream>
#include <array>
#include <string>
#include <vector>
#include <map>
#include <cstddef>
#include <cstring>
#include <memory>
#include <cassert>
#include "unistd.h"
#include <tuple>

using namespace std;

int factorial(int n)
{
    int result = 1;
    for(int i = 2; i <=n; ++i) result *= i;
    return result;
}

template <typename T>
struct MemoryBlock{
	MemoryBlock(){}
	std::array<std::byte, sizeof(T)> memory; 
};

template <typename T>
class Pool{
public:
	std::tuple<MemoryBlock<T>*,bool*> createBuffer(size_t SIZE)
	{
		MemoryBlock<T>* buffer=(MemoryBlock<T>*)(malloc(sizeof(MemoryBlock<T>) * SIZE));
		bool* occupied=(bool*)(malloc(SIZE));
		for(size_t i=0; i<SIZE; ++i) occupied[i]=false;
		return std::make_tuple(buffer,occupied);
	}
	Pool(size_t _size):capacity(_size),size(0)
	{
		const auto tup=createBuffer(capacity);
		buffers.push_back ( std::get<0>(tup) );
		occupied.push_back( std::get<1>(tup) );
		sizes.push_back(capacity);
		counters.push_back(0);
	}
	~Pool()
	{
	  //cout<<"~Pool()"<<endl;
		for(size_t i=0; i<buffers.size(); ++i) if(buffers.at(i)) free(buffers.at(i));
		for(size_t i=0; i<occupied.size(); ++i) if(occupied.at(i)) free(occupied.at(i));
	}
	//
	T* allocate(size_t n)
	{
/*
		cout<<"Pool::allocate("<<n<<")   buffers.size()="<<buffers.size()<<endl;
		cout<<"sizes of buffers:";
		for(size_t i=0; i<buffers.size(); ++i) cout<<sizes.size()<<" ";
		cout<<endl;
*/
		int LastBufferIndex=buffers.size()-1;
		int index=find_vacant_block( LastBufferIndex, n );
		if(-1 == index)
		{
			reallocate();
			LastBufferIndex=buffers.size()-1;
			index=find_vacant_block(LastBufferIndex, n);
		}
		for(size_t j=0; j<n; ++j) occupied.at(LastBufferIndex)[counters.at(LastBufferIndex)+j]=true;
	  //cout<<"@#@ LastBufferIndex="<<LastBufferIndex<<" index="<<index<<endl;
		MemoryBlock<T>* ptr = get(LastBufferIndex, index);
		//size+=n;
/*
		cout<<"counters.at("<<LastBufferIndex<<") before update="<<counters.at(LastBufferIndex)<<endl;
*/
		counters.at(LastBufferIndex) += n;
/*
		cout<<"counters.at("<<LastBufferIndex<<") after update="<<counters.at(LastBufferIndex)<<endl;
*/
        if(ptr) return reinterpret_cast<T*>(ptr);
        throw std::bad_alloc();
	}
	//if not enough space, resize as: new_size=old_size*2+1
	void reallocate()
	{
		int old_capacity=capacity;
		(void)old_capacity;
		capacity=capacity*2+1;
/*
		cout<<"REALLOC: old_capacity"<<old_capacity
		    <<" new_capacity="<<capacity
		    <<endl;
*/
		const auto tup=createBuffer(capacity);
		buffers.push_back ( std::get<0>(tup) );
		occupied.push_back( std::get<1>(tup) );
		sizes.push_back(capacity);
/*
		cout<<"sizes of buffers: capacity="<<capacity<<" sizes.size()="<<sizes.size()<<"   ";
		for(size_t i=0; i<buffers.size(); ++i) cout<<sizes.at(i)<<" ";
		cout<<endl;
*/
		counters.push_back(0);
	}
	void deallocate(T* ptr)
	{
		(void) ptr;
	}
	int find_vacant_block(int BufferIndex, int n)
	{
		int BlockIndex=-1;
		for(int i=counters.at(BufferIndex); i<sizes.at(BufferIndex); ++i)
		{
			if( !occupied.at(BufferIndex)[i] )
			{
				bool flag=true;
				for(int j=1; j<n; ++i)
				{
					if( occupied.at(BufferIndex)[i+j] ){flag=false; break;}
				}
				if(flag)
				{
					BlockIndex=i;
					break;
				}
				else continue;
			}
		}
	  //cout<<"Pool::find_vacant_block("<<BufferIndex<<", "<<n<<"):   "<<endl;
		return BlockIndex;
	}
	//return a raw pointer to an "index" item in the array "buffer": 
	MemoryBlock<T>* get(int BufferIndex, int index) const
	{
		return &buffers.at( BufferIndex )[ index ];
	}
private:
	static const int allocation_number=0;
	size_t capacity;
	size_t size;

	vector<int> counters;
	vector<int> sizes;
	vector<bool*> occupied;
	vector<MemoryBlock<T>*> buffers;
};


template <typename T>
class SimpleAllocator {
public:
    using value_type = T;

    SimpleAllocator() noexcept 
    {
    	pool=new Pool<T>(2);

    }
    ~SimpleAllocator()
    {
    	delete pool;
    }
    
    template <typename U>
    SimpleAllocator(const SimpleAllocator<U>& other) {
        (void) other;
    }
    
    T* allocate(size_t n) {
        auto ptr = (T*)pool->allocate(n);//static_cast<T*>(malloc(sizeof(T) * n));
	  //cout<<"SimpleAllocator::allocate("<<n<<"):   address="<<ptr<<endl;
        if (ptr) return ptr;
        throw std::bad_alloc();
    }
    
    void deallocate(T* ptr, size_t n) {
        (void) n;
        (void) ptr;
        pool->deallocate(ptr);
    }
    template <typename U, typename... Args>
	void construct(U *p, Args &&...args){
		new (p) U(std::forward<Args>(args)...);
	}
	template <typename U>
	void destroy(U *p) {p->~U();}
private:
    Pool<T>* pool; 
};

template <typename T, typename U>
bool operator==(const SimpleAllocator<T>& a1, const SimpleAllocator<U>& a2) {
    (void) a1; (void) a2;
    return true;
}

template <typename T, typename U>
bool operator!=(const SimpleAllocator<T>& a1, const SimpleAllocator<U>& a2) {
    (void) a1; (void) a2;
    return false;
}


int main()
{
	//1//
	std::map<int,int> m1;
    for(int i=0; i<=9; ++i) m1[i]=factorial(i);
    for(auto [x,y] : m1) cout<<x<<" "<<y<<endl;
    //2//
    std::map<int, int, std::less<int>, SimpleAllocator<std::pair<const int,std::string>>> m2;
	for(int i=0; i<=10; ++i) m2[i]=factorial(i);
	int i=0;
    for(auto it=m2.begin(); it!=m2.end(); ++it)
    {
    	i++;
    	cout<<it->first<<" "<<it->second<<endl;
	}
	return 0;
}



// https://sjbrown.co.uk/posts/pooled-allocators-for-the-stl/
