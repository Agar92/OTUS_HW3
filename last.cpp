#include <iostream>
#include <cstring>
#include <cassert>

//
//Pool memory allocator example
//

using std::cout;
using std::endl;

struct P{
  P(int X, int Y, int Z):x(X),y(Y),z(Z){}
  P(const P& p):P(p.x,p.y,p.z){}
  int x, y, z;
  friend std::ostream& operator<<(std::ostream& out, const P & p)
  {
    out<<p.x<<" "<<p.y<<" "<<p.z<<endl;;
    return out;
  }
};

template <typename T>
class Pool{
  static const int blockSize=2;
public:
  struct block{
    block():next(nullptr){}
    block* next;
    uint8_t bytes[sizeof(T)];
  };
  Pool()
  {
    data=new block[3];
  }
  ~Pool()
  {
    delete [] data;
  }
  P* get(int index)
  {
    return reinterpret_cast<P*>(&data[index].bytes);
  }
  const P* get(int index) const
  {
    return reinterpret_cast<const P*>(data[index].bytes);
  }
  void allocate(const P & p)
  {
    if( getFreeBlocksCount() )
    {
      block* temp=freeBlock;
      freeBlock=freeBlock->next;
      new ( reinterpret_cast<P*>(&temp->bytes) ) P(p);
      return;
    }
  //block* ptr=reinterpret_cast<block*>( get(counter) );
    block* ptr=&data[counter];
    new (ptr->bytes) P(p);
    cout<<"@@@: "<<*reinterpret_cast<P*>(ptr->bytes)<<endl;
    int index=-1;
    for(int i=0; i<=counter; ++i)
    {
      if(ptr == &data[i])
      {
        index=i;
        break;
      }
    }
    cout<<"index="<<index<<endl;
    //cout<<"$0$: "<<*reinterpret_cast<P*>(&data[0].bytes)<<endl;
    //cout<<"$1$: "<<*reinterpret_cast<P*>(&data[1].bytes)<<endl;
    //cout<<"$2$: "<<*reinterpret_cast<P*>(&data[2].bytes)<<endl;
    counter++;
  }
  void deallocate(int index)
  {
    assert(index<counter);
/*
    cout<<"deallocate():"<<endl;
    for(int i=0; i<counter; ++i) cout<<*get(i);
    cout<<endl;
    cout<<"data["<<index<<"]="<<*((P*)&data[index])<<endl;
    cout<<"data["<<0<<"]="<<*reinterpret_cast<P*>(&data[0])<<endl;
    cout<<"data["<<1<<"]="<<*reinterpret_cast<P*>(&data[1])<<endl;
    cout<<"data["<<2<<"]="<<*reinterpret_cast<P*>(&data[2])<<endl;
    cout<<"index="<<index<<" counter="<<counter
        <<" (counter-index-1)="<<(counter-index-1)
        <<" sizeof(P)="<<sizeof(P)
        <<" sizeof(block)="<<sizeof(block)
        <<endl;
    std::memcpy(&data[index], &data[(index+1)], (counter-index-1)*sizeof(block));
    counter--;
    cout<<"AFTER:"<<endl;
    cout<<"data["<<0<<"]="<<*reinterpret_cast<P*>(&data[0])<<endl;
    cout<<"data["<<1<<"]="<<*reinterpret_cast<P*>(&data[1])<<endl;
    cout<<"data["<<2<<"]="<<*reinterpret_cast<P*>(&data[2])<<endl;
*/
    
//*
    block* next=freeBlock;
    freeBlock=&data[index];
    if(freeBlock) freeBlock->next=next;
    //if(freeBlock) cout<<"next="<<next<<" freeBlock="<<freeBlock<<" freeBlock->next="<<freeBlock->next<<endl;
//*/
  }
  int getFreeBlocksCount() const
  {
    block* temp=freeBlock;
    int count=0;
    while(temp!= nullptr)
    {
      temp=temp->next;
      count++;
    }
    return count;
  }
  friend std::ostream& operator<<(std::ostream& out, const Pool& p)
  {
    for(int i=0; i<p.counter; ++i) out<<*reinterpret_cast<P*>(&p.data[i].bytes);
    out<<endl;
    return out;
  }
  block* freeBlock=nullptr;
  int counter=0;
  block* data;
};


struct brick{
  uint8_t data[100];
};

/*
class C{
  struct FreeNode{
    FreeNode():next(nullptr), index(-1){}
    FreeNode* next;
    int index;
  };
public:
  C()
  {
    const char * string="vacant";
    for(int i=0; i<3; ++i)
    {
      for(int j=0; j<strlen(string); ++j) bricks[i].data[j]=string[j];
      bricks[i].data[strlen(string)]='\0';
    }
    for(int i=0; i<3; ++i)
    {
      char name[strlen(string)];
      std::memcpy(name, &bricks[i].data[0], strlen(string));
      printf("i=%d %s\n", i, name);
      for(int j=0; j<strlen(string); ++j) cout<<name[j]<<endl;
    }
  }
  uint8_t* get(int index)
  {
    int ni=index/3;
    int nj=index%3;
    C* temp=this;
    for(int i=0; i<ni; ++i) temp=temp->next;
    return reinterpret_cast<uint8_t*>( &temp[nj] );
      
  }
private:
  C* next;
  brick bricks[3];
};
*/
int main()
{
  
  exit(0);
  Pool<P> pool;
  pool.allocate({1,2,3});
  pool.allocate({4,5,6});
  pool.allocate({7,8,9});
  P* ptr1 = pool.get(0);
  cout<<*ptr1<<endl;

  cout<<"pool.getFreeBlocksCount()="<<pool.getFreeBlocksCount()<<endl;
  cout<<pool<<endl;

  pool.deallocate(2);
  pool.deallocate(0);

  cout<<"pool.getFreeBlocksCount()="<<pool.getFreeBlocksCount()<<endl;
  cout<<pool<<endl;
  
  pool.allocate({1,0,-1});
  pool.allocate({27,30,-100});

  cout<<"pool.getFreeBlocksCount()="<<pool.getFreeBlocksCount()<<endl;

  cout<<pool<<endl;
  
  return 0;
}
