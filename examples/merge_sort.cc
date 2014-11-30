#include <cstdio>
#include <algorithm>
#include <random>
#include <ctime>
#include "../include/thread_pool.h"

using namespace std;
using namespace BoboThreadd;

// Let's prove the importance of multi-threading with ThreadPool
// we'll try to speed up sorting without any algorithmic tricks.

// I'm going to compare execution time of
// array sorting 
// 1 thread vs 2 threads

// Since I'm using MergeSort, the Task* is - Merge two sorted segments
// Task* contains positions, not value copies
// And it changes actual array values, so we can't execute tasks
// with Intersecting segments (moreover, it will make algorithm wrong).

template<typename ItemType>
class MergeSegments : public Task {
public:  
  
  // every task must contain 3 pointers to return indexes
  MergeSegments(vector<ItemType>* data, 
                vector<ItemType>* help,
                size_t left, 
                size_t mid,
                size_t right)
    : done_(false), 
      data_(data),
      help_(help),
      left_(left),
      mid_(mid),
      right_(right) {

  }

  virtual void work() {          
        
    size_t i = left_, j = mid_, ind = left_;

    // merge left..mid-1, mid..right-1
    // this code will be optimized better than std::merge + std::copy
    while ( i < mid_ && j < right_ )
      if ( help_->at(i) < help_->at(j) )
        data_->at(ind++) = help_->at(i++);
      else
        data_->at(ind++) = help_->at(j++);
    while ( i < mid_ )
      data_->at(ind++) = help_->at(i++);
    while ( j < right_ )
      data_->at(ind++) = help_->at(j++);

    done_ = true;
  }

  virtual bool done() {
    return done_;
  }

private:  
  bool done_;
  vector<ItemType>* data_;
  vector<ItemType>* help_;
  size_t left_;
  size_t mid_;
  size_t right_;
};

template<typename ItemType>
void MergeSort(vector<ItemType>& array, 
               vector<ItemType>& helpArray, 
               int size, ThreadPool *pool)
{
    // I took some of this code from very interesting article
    // http://warp.povusers.org/SortComparison/
    // you can look at the original code to fix possible missunderstanding

    vector<ItemType>* src = &array;      // source
    vector<ItemType>* dst = &helpArray;  // destination
    
    vector< MergeSegments<ItemType>* > mem;

    pool->start();

    for(int bSize = 2; bSize < size*2; bSize *= 2)
    {
      
      mem.clear(); mem.reserve(size / bSize);
      
      int dstInd = 0;

      // it's a good tone of using ThreadPool to suspend threads
      // before adding tasks (to avoid useless synchronizations)
      pool->suspend();
      for(int bInd = 0; bInd < size; bInd += bSize)
      {
        int left   = bInd;
        int middle = bInd + bSize/2;
        int right  = bInd + bSize;
        if (middle > size) middle = size;
        if (right > size)  right = size;

        mem.push_back(new MergeSegments<ItemType>(
          dst, src, left, middle, right));

        pool->execute(mem.back());           
      }
      pool->start();
      pool->wait();

      swap(src,dst);

      for (auto task : mem)
        delete task;      
    }
    if(src == &helpArray)
    {
      for (int i = 0; i < size; ++i)
        dst->at(i) = src->at(i);
    }
}

int main () {

  const int sz = 1000*1000;

  // generate array
  vector<int> arr(sz);
  uniform_int_distribution<int> rnd(1, max(1, sz/3));
  default_random_engine r;
  for (auto &a : arr) 
    a = rnd(r);
  
  // let's measure the execution time
  typedef chrono::high_resolution_clock Clock;
  typedef chrono::duration<double>      Duration;
  auto tm = Clock::now(); Duration elapsed_sec; 

  vector<int> std_result(arr);
  sort(begin(std_result), end(std_result));
  
  elapsed_sec = chrono::duration_cast<Duration>(Clock::now() - tm);
  printf("std::sort executed in %.3f sec\n", elapsed_sec.count());    
  
  // Now let's see how we can speed up our algorithm by 
  // executing tasks in parallel

  // I'm trying to avoid hashing memory and 
  // get full control on allocation.
  // But compiler will do realloc with -O2
  // (according to -fdump-free-optimized option result)
  vector<int>* tmp;
  ThreadPool* pool;
  vector<int> merge_result;

  // 1 thread

  tm = Clock::now();

  int cnt_1 = 1; 
  tmp = new vector<int>(sz);
  pool = new ThreadPool(cnt_1, ThreadPool::kConsecutive);
  merge_result.assign(begin(arr), end(arr));
  MergeSort<int>(merge_result, *tmp, sz, pool);
  delete pool;
  delete tmp; 
  
  elapsed_sec = chrono::duration_cast<Duration>(Clock::now() - tm);
  printf("MergeSort : %d threads executed in %.3f sec\n", 
    cnt_1, elapsed_sec.count());
  
  printf( (merge_result == std_result) ? "TEST PASSED\n" : "TEST FAILED\n" );

  // 2 threads
    
  tm = Clock::now();
  int cnt_2 = 2;
  tmp = new vector<int>(sz);
  pool = new ThreadPool(cnt_2, ThreadPool::kConsecutive);  
  merge_result.assign(begin(arr), end(arr));
  MergeSort<int>(merge_result, *tmp, sz, pool);
  delete pool;
  delete tmp; 

  elapsed_sec = chrono::duration_cast<Duration>(Clock::now() - tm);
  printf("MergeSort : %d threads executed in %.3f sec\n", 
    cnt_2, elapsed_sec.count());
  
  printf( (merge_result == std_result) ? "TEST PASSED\n" : "TEST FAILED\n" );

  // second one should win on computers with 2+ processors  
  // without any dependency on compiler or platform
  // 2 threads always perform better for this constraints
  // but you're always may get much better (or much worse) 
  // perfomance while testing different counts of threads and
  // dispatch modes

  return 0;
}
