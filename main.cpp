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
	Pool(size_t _size):capacity(_size),size(0)
	{
		buffer=(MemoryBlock<T>*)(malloc(sizeof(MemoryBlock<T>) * capacity));
		occupied=(bool*)(malloc(capacity));
		for(size_t i=0; i<capacity; ++i) occupied[i]=false;
	}
	~Pool()
	{
		cout<<"~Pool()"<<endl;
		//free(buffer);
		//free(occupied);
	}
	//
	T* allocate(size_t n)
	{
		int index=find_vacant();
		if(-1 == index)
		{
			reallocate();
			index=find_vacant();
		}
		for(size_t j=0; j<n; ++j) occupied[size+j]=true;
		MemoryBlock<T>* ptr = get(index);
		size+=n;
        if(ptr) return reinterpret_cast<T*>(ptr);
        throw std::bad_alloc();
	}
	//if not enough space, resize as: new_size=old_size*2+1
	void reallocate()
	{
		const int new_capacity=capacity*2+1;
		new_buffer=(MemoryBlock<T>*)(malloc(sizeof(MemoryBlock<T>) * new_capacity));
		//std::memcpy(new_buffer, buffer, size*sizeof(MemoryBlock<T>));
		//free(buffer);
	  //buffer=new_buffer;
		new_occupied=(bool*)(malloc(sizeof(bool) * new_capacity));
		//std::memcpy(new_occupied, occupied, size*sizeof(bool));
		for(size_t i=size; i<(size_t)new_capacity; ++i) occupied[i]=false;
		//free(occupied);
	  //occupied=new_occupied;
		capacity=new_capacity;
	}
	void deallocate(T* ptr)
	{
		(void) ptr;
	}
	//find a first vacant MemoryBlock in the array "buffer" from left to right for filling it in with the data:
	int find_vacant()
	{
		int index=-1;
		for(int i=0; i<(int)capacity; ++i)
		{
			if(!occupied[i])
			{
				index=i;
				break;
			}
		}
		return index;
	}
	//return a raw pointer to an "index" item in the array "buffer": 
	MemoryBlock<T>* get(int index) const
	{
		if(index<2)
			return &buffer[index];
		else
			return &new_buffer[index-2];
	}
private:
	static const int allocation_number=0;
	size_t capacity;
	size_t size;
	bool * occupied;
	MemoryBlock<T> * buffer;
	MemoryBlock<T> * new_buffer;
	bool * new_occupied;
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
	cout<<"operator==(const SimpleAllocator<T>& a1, const SimpleAllocator<U>& a2)"<<endl;
    (void) a1; (void) a2;
    return true;
}

template <typename T, typename U>
bool operator!=(const SimpleAllocator<T>& a1, const SimpleAllocator<U>& a2) {
	cout<<"operator!=(const SimpleAllocator<T>& a1, const SimpleAllocator<U>& a2)"<<endl;
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
    cout<<"START:"<<endl;
    std::map<int, int, std::less<int>, SimpleAllocator<std::pair<const int,std::string>>> m2;
    cout<<"START FILLING THE MAP:"<<endl;
	for(int i=0; i<=10; ++i)
	{
		m2[i]=factorial(i);
		cout<<"m2["<<i<<"]="<<m2[i]<<" factorial(i)="<<factorial(i)<<endl;
	}
	cout<<"ENTER"<<endl;
	int i=0;
////auto it=m2.begin();
    for(auto it=m2.begin(); it!=m2.end(); ++it)
////for(int i=0; i<3; ++i)
    {
////////std::next(it);
    	i++;
    	cout<<"#"<<i<<":"<<endl;
    	cout<<it->first<<" "<<it->second
    	    <<" | "<<m2.begin()->first<<" "<<m2.begin()->second
    	    <<" | "<<m2.end()->first<<" "<<m2.end()->second
    	    <<endl;
			//sleep(1);
    }
    cout<<"EXIT"<<endl;
    //3//
	

	return 0;
}



// https://sjbrown.co.uk/posts/pooled-allocators-for-the-stl/
