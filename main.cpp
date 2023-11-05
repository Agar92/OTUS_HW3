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
struct Buffer{
	Buffer(int SIZE, Buffer* NEXT):capacity(SIZE),next(NEXT)
	{
		data=new MemoryBlock<T>[capacity];
		occupied=new bool[capacity];
		for(int i=0; i<this->capacity; ++i) this->occupied[i]=false;
	}
	~Buffer()
	{
		delete [] data;
		delete [] occupied;
	}
	int size=0;
	int capacity;
	Buffer* next=nullptr;
	bool* occupied;
	MemoryBlock<T>* data;
};

template <typename T>
class Pool{
public:
	Pool(size_t _size):capacity(_size),size(0)
	{
		buffer = new Buffer(capacity, buffer);
/*
		Buffer<T>* buffer_ptr=buffer;
		cout<<"Check buffer array:"<<endl;
		while(buffer_ptr != nullptr)
		{
			cout<<buffer_ptr<<" ";
			buffer_ptr=buffer_ptr->next;
		}
		cout<<endl;
*/
	}
	~Pool()
	{
		//cout<<"~Pool() buffer="<<buffer<<endl;
		Buffer<T>* buffer_ptr=buffer;
		while(buffer_ptr != nullptr)
		{
			Buffer<T>* temp=buffer_ptr;
			buffer_ptr=buffer_ptr->next;
			delete temp;
		}
	}
	//
	T* allocate(size_t n)
	{
/*
		cout<<"============Pool::allocate("<<n<<")   "<<endl;
		cout<<"buffer="<<buffer<<" buffer->next="<<buffer->next<<endl;
		cout<<"E$1$E"<<endl;
		cout<<"Check buffers: ";
		cout<<"E$2$E"<<endl;
		Buffer<T>* bptr=buffer;
		while(bptr != nullptr)
		{
			cout<<bptr<<"|next="<<bptr->next;
			bptr=bptr->next;
		}
		cout<<endl;
		cout<<"Check buffer capacities: ";
		bptr=buffer;
		while(bptr != nullptr)
		{
			cout<<bptr->capacity<<" ";
			bptr=bptr->next;
		}
		cout<<endl;
		cout<<"Check buffer sizes: ";
		bptr=buffer;
		while(bptr != nullptr)
		{
			cout<<bptr->size<<" ";
			bptr=bptr->next;
		}
		cout<<endl;
*/
	  //exit(0);
		int BlockIndexInBuffer=find_vacant_block(n);
		//cout<<"#@1@# BlockIndexInBuffer="<<BlockIndexInBuffer<<endl;
		if(-1 == BlockIndexInBuffer)
		{
			//cout<<"REALLOC:"<<endl;
			capacity=2*capacity+1;
			Buffer<T>* save=buffer;
			Buffer<T>* new_buffer = new Buffer<T>(capacity, buffer);
			buffer=new_buffer;
			buffer->next=save;
			BlockIndexInBuffer=find_vacant_block(n);
			//cout<<"#@2@# BlockIndexInBuffer="<<BlockIndexInBuffer<<endl;
		}
		//cout<<"BEFORE get()"<<endl;
		//CheckBuffer();
		MemoryBlock<T>* ptr = get(BlockIndexInBuffer);
		//cout<<"AFTER get()"<<endl;
		//CheckBuffer();
		for(size_t i=0; i<n; ++i) buffer->occupied[buffer->size+i]=true;
		buffer->size += n;
        if(ptr) return reinterpret_cast<T*>(ptr);
        throw std::bad_alloc();
	}
	//if not enough space, resize as: new_size=old_size*2+1
	void reallocate()
	{

	}
	void deallocate(T* ptr)
	{
		(void) ptr;
	}

	void CheckBuffer()
	{
		Buffer<T>* buffer_ptr1=buffer;
		cout<<"-------Check buffer array: buffer_ptr1="
			<<buffer_ptr1<<" buffer_ptr1->next="
			<<buffer_ptr1->next
			<<endl;
		usleep(100000);
		while(buffer_ptr1 != nullptr)
		{
			cout<<buffer_ptr1<<" ";
			buffer_ptr1=buffer_ptr1->next;
		}
		cout<<endl;
		//
		buffer_ptr1=buffer;
	}

	int find_vacant_block(int n)
	{
      //cout<<"BEGIN Pool::find_vacant_block("<<n<<")   "<<endl;
		(void)n;
		int BlockIndexInBuffer=-1;

		//CheckBuffer();

/*
		for(int i=0; i<buffer->capacity; ++i)
		{
			cout<<std::boolalpha<<buffer->occupied[i]<<" ";
		}
		cout<<endl;
*/
		for(int i=0; i<buffer->capacity; ++i)
		{
			if( !buffer->occupied[i])
			{
				BlockIndexInBuffer=i;
				break;
			}
		}

		//CheckBuffer();
/*
		cout<<"END Pool::find_vacant_block("<<n<<")   "
		    <<" BlockIndexInBuffer="<<BlockIndexInBuffer
		    <<endl;
*/
		return BlockIndexInBuffer;
	}
	//return a raw pointer to an "index" item in the array "buffer": 
	MemoryBlock<T>* get(int BlockIndexInBuffer)
	{
		//cout<<"<<<<<<<<Pool::get("<<buffer<<","<<BlockIndexInBuffer<<")"<<endl;
		assert(BlockIndexInBuffer >-1 && BlockIndexInBuffer < buffer->capacity);
		return &buffer->data[ BlockIndexInBuffer ];
	}
private:
	size_t capacity;
	size_t size;
	Buffer<T>* buffer=nullptr;
};


template <typename T>
class SimpleAllocator {
public:
    using value_type = T;

    SimpleAllocator() noexcept 
    {
    	pool=new Pool<T>(1);

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
        auto ptr = (T*)pool->allocate(n);
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
	//cout<<"ENTER:"<<endl;
    std::map<int, int, std::less<int>, SimpleAllocator<std::pair<const int,std::string>>> m2;
	for(int i=0; i<=10; ++i)
	{
		m2[i]=factorial(i);
		//cout<<"m2["<<i<<"]="<<m2[i]<<"  factorial("<<i<<")="<<factorial(i)<<endl;
	}
	int i=0;
    for(auto it=m2.begin(); it!=m2.end(); ++it)
    {
    	i++;
    	cout<<it->first<<" "<<it->second<<endl;
	}
	return 0;
}



// https://sjbrown.co.uk/posts/pooled-allocators-for-the-stl/
