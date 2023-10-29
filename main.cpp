#include <iostream>
#include <array>
#include <string>
#include <vector>
#include <map>
#include <cstddef>
#include <cstring>
#include <memory>
#include <cassert>

using namespace std;

int factorial(int n)
{
    int result = 1;
    for(int i = 2; i <=n; ++i) result *= i;
    return result;
}

template <typename T>
struct MemoryBlock{
	MemoryBlock()
	{
		cout<<"sizeof(this)="<<sizeof(this)<<" "
		    <<"sizeof(*this)="<<sizeof(*this)<<" "
		    <<"sizeof(T)="<<sizeof(T)<<" "
		    <<"sizeof(string)="<<sizeof(std::string)
		    <<endl;
	}
	std::array<std::byte, sizeof(T)> memory; 
};

template <typename T>
class Pool{
public:
	Pool(size_t _size):capacity(_size),size(0)
	{
		cout<<"Pool()"<<endl;
		buffer=static_cast<MemoryBlock<T>*>(malloc(sizeof(MemoryBlock<T>) * capacity));
		occupated=new bool[capacity];
		for(size_t i=0; i<capacity; ++i) occupated[i]=false;
		//buffer=::operator new(size*sizeof(MemoryBlock<T>));
	}
	~Pool()
	{
		cout<<"~Pool()"<<endl;
		//delete [] buffer;
		//delete [] occupated;
	}
	//
	void* allocate()
	{
		int index=find_vacant();
		if(-1 == index)
		{
			reallocate();
			index=find_vacant();
		}
		size++;
		cout<<"Pool::allocate size="<<size<<" index="<<index<<" capacity="<<capacity<<endl;
		occupated[index]=true;
		cout<<"1 index="<<index<<endl;
		MemoryBlock<T>* ptr = get(index);
        if(ptr) return ptr;
        throw std::bad_alloc();
	}
	//if not enough space, resize as: new_size=old_size*2+1
	void reallocate()
	{
		cout<<"Before Pool::reallocate size="<<size<<" capacity="<<capacity<<endl;
		for(int i=0; i<10; ++i) cout<<"===";
		cout<<endl;
		cout<<"REALLOC:"<<endl;
		for(int i=0; i<10; ++i) cout<<"===";
		cout<<endl;
		const int new_capacity=capacity*2+1;
		MemoryBlock<T> * new_buffer =
			static_cast<MemoryBlock<T>*>(malloc(sizeof(MemoryBlock<T>) * new_capacity));
		std::memcpy(new_buffer, buffer, size*sizeof(MemoryBlock<T>));
		free(buffer);
		buffer=new_buffer;
		//
		bool * new_occupated =
			static_cast<bool*>(malloc(sizeof(bool) * new_capacity));
		std::memcpy(new_occupated, occupated, size*sizeof(bool));
		free(occupated);
		occupated=new_occupated;
		capacity=new_capacity;
		cout<<"After Pool::reallocate size="<<size<<" capacity="<<capacity<<endl;
	}
	void deallocate(T* ptr)
	{
		(void) ptr;
		//delete ptr;
	}
	//find a first vacant MemoryBlock in the array "buffer" from left to right for filling it in with the data:
	int find_vacant()
	{
		cout<<"Before Pool::find_vacant size="<<size<<endl;
		int index=-1;
		for(int i=0; i<(int)capacity; ++i)
		{
			if(!occupated[i])
			{
				index=i;
				break;
			}
		}
		cout<<"After Pool::find_vacant size="<<size<<" index="<<index<<endl;
		return index;
	}
	//return a raw pointer to an "index" item in the array "buffer": 
	MemoryBlock<T>* get(int index) const
	{
		cout<<"Pool::get"<<endl;
		cout<<"2.1 index="<<index<<endl;
		assert(index > -1 && index < (int)size);
		cout<<"2.2 index="<<index<<endl;
		return &buffer[index];
	}
private:
	static const int allocation_number=0;
	size_t capacity;
	size_t size;
	bool * occupated;
	MemoryBlock<T> * buffer;
};


template <typename T>
class SimpleAllocator {
public:
    using value_type = T;

    SimpleAllocator()
    {
    	cout<<__FUNCTION__<<endl;
    	pool=new Pool<T>(2);

    }
    ~SimpleAllocator()
    {
    	cout<<__FUNCTION__<<endl;
    	delete pool;
    }
    
    template <typename U>
    SimpleAllocator(const SimpleAllocator<U>& other) {
    	cout<<__FUNCTION__<<endl;
        (void) other;
    }
    
    T* allocate(size_t n) {
    	std::cout<<"SimpleAllocator::allocate"<<" n="<<n<<std::endl;
        auto ptr = (T*)pool->allocate();//static_cast<T*>(malloc(sizeof(T) * n));
        if (ptr) return ptr;
        throw std::bad_alloc();
    }
    
    void deallocate(T* ptr, size_t n) {
    	cout<<__FUNCTION__<<endl;
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
    cout<<"START:"<<endl;
    std::map<int, int, std::less<int>, SimpleAllocator<std::pair<const int,std::string>>> m2;
    cout<<"START FILLING THE MAP:"<<endl;
	for(int i=0; i<=2; ++i) m2[i]=factorial(i);
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
    }
    cout<<"EXIT"<<endl;
    //3//
	

	return 0;
}
