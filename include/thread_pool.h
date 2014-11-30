/*
* Copyright (c) 2014, Zakharov Konstantin
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to 
* deal in the Software without restriction, including without limitation the 
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
* sell copies of the Software, and to permit persons to whom the Software is 
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in 
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*
*/

#ifndef BBTHREADD_THREADPOOL_H_
#define BBTHREADD_THREADPOOL_H_

#include <vector>

#include "worker.h"
#include "task.h"

namespace BoboThreadd {

// Spawns a set of threads that are used to run submitted tasks in parallel
class ThreadPool {
public:

  // Create n Workers
  // ( n is number of threads to service tasks with )	
  ThreadPool(size_t n = 1, int dispatch_type = kConsecutive);

  // Destroy all workers (stop threads)
  ~ThreadPool();
  
  // Choose the method that makes ThreadPool perfomance better on your tasks
  enum DispatchMethod {
    // give tasks for workers in order 0,1,..,n-1,0,1,..,n-1
    kConsecutive = 0,  
    // use random worker index uniformly distributed in [0; n-1]
    kRandomized = 1,  
    // find worker with lowest count of queued tasks
    kGreedy = 2,
    // using heuristics + some sort of kGreedy, kRandomized
    kCombination = 3
  };  
  
  void execute(Task* task); // Submit task for parallel execution
  void start(); // Start executing tasks or cancel suspend()  
  size_t size(); // Returns count of workers

  // Blocks calling thread until all tasks 
  // submitted prior to this invocation complete
  void wait();

  // Makes workers sleeping  
  void suspend();    // NOTE: currently running tasks unaffected

  // Removes tasks that had been submitted prior to this function invoking.
  // Tasks submitted after the invocation of this function are unaffected.
  void interrupt();  // NOTE: currently running tasks unaffected    
  
private:
  // 1 Worker = 1 std::thread + 1 std::queue<Task*> + 1 std::mutex
  std::vector<Worker*> workers_;

  // TODO: create class "Dispatcher" with method "int next()",
  //       provide an interface allowing user to implement his own dispatcher
  int dispatch_type_;
  int current_index_;

  int get_consecutive();  // kConsecutive
  int get_randomized();   // kRandomized
  int get_greedy();       // kGreedy
  int get_combination();  // kCombination
};

}  // namespace BoboThreadd

#endif // BBTHREADD_THREADPOOL_H_
