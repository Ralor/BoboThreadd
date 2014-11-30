#include <cstdio>
#include <algorithm>
#include <vector>
#include <random>
#include <chrono>

#include "../include/thread_pool.h"

using namespace std;
using namespace BoboThreadd;

class ArrayGenerator : public Task {
public:
  int n, a;
  bool generated;
  vector<int>* result;

  ArrayGenerator(int n, int a) : n(n), a(max(a,1)), generated(false) { }

  void work() {

    std::default_random_engine r;
    std::uniform_int_distribution<int> rnd(1,a);

    result = new vector<int>();
    result->reserve(n);
    for (int i = 0; i < n; ++i)
      result->push_back(rnd(r));

    generated = true;
  }

  bool done() {
    return generated;
  }
};



int main () {

  // we need to compare two methods - with/without ThreadPool
  // even when multithreading is not the only way to solve a problem
  
  // let's generate "n" random arrays with lengths "m" and amplitude "1..a"

  const int n = 60, m = 10000, a = 1000;

  double ev; // math expectation should be near a/2

  // we'll benchmark them a little
  typedef chrono::high_resolution_clock Clock;
  typedef chrono::duration<double>      Duration; 
  Duration elapsed_sec;

  // linear code starting  
  auto tm = Clock::now(); 

  // stack array of pointers
  vector< vector<int>* > lin_arrs;
  lin_arrs.reserve(n);
  for(int i = 0; i < n; ++i) {
    std::default_random_engine r;
    std::uniform_int_distribution<int> rnd(1,max(1,a));

    // generate arrays in heap
    vector<int>* arr = new vector<int>(); 
    arr->reserve(m);
    for(int j = 0; j < m; ++j)
      arr->push_back(rnd(r));
    lin_arrs.push_back(arr);

    // sort array
    sort(begin(*lin_arrs.back()), end(*lin_arrs.back()));
  }

  ev = 0;
  for (auto arr : lin_arrs)
    for (auto num : *arr)
      ev += num;
  ev /= n * m;

  for (auto arr : lin_arrs) 
    delete arr;

  elapsed_sec = chrono::duration_cast<Duration>(Clock::now() - tm);
  printf("Amplitude is 1..%d, Math Expectation : %f\n", a, ev);
  printf("Linear code executed in %.3f sec\n", elapsed_sec.count());	

  // ThreadPool starting
  tm = Clock::now();

  ThreadPool pool(2, ThreadPool::kConsecutive); 
  
  pool.start();  
  // stack array of pointers
  vector< ArrayGenerator* > pool_arrs; 
  pool_arrs.reserve(n);
  // generate arrays in heap
  for (int i = 0; i < n; ++i) {    
    ArrayGenerator* p = new ArrayGenerator(m,a);
    pool_arrs.push_back(p);    
    pool.execute(p);
  }
  pool.wait();

  ev = 0;
  for (auto arr : pool_arrs)
    for (auto num : *arr->result)
      ev += num;
  ev /= n * m;

  for (auto arr : pool_arrs) 
    delete arr;

  elapsed_sec = chrono::duration_cast<Duration>(Clock::now() - tm);
  printf("Amplitude is 1..%d, Math Expectation : %f\n", a, ev);
  printf("ThreadPool executed in %.3f sec\n", elapsed_sec.count());

  return 0;
}
