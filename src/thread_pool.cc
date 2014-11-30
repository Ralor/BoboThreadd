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

#include "../include/thread_pool.h"
#include <random>

using namespace BoboThreadd;

ThreadPool::ThreadPool(size_t n, int dispatch_type)
  : current_index_(0),
    dispatch_type_(dispatch_type){ 						
  workers_.reserve(n);
  for (size_t i = 0; i < n; ++i)
    workers_.push_back(new Worker());  
}

ThreadPool::~ThreadPool() {
  for (auto worker : workers_)
    delete worker;
}

void ThreadPool::execute(Task* task) {
  int chosen_index = -1;

  switch (dispatch_type_) {
    case kConsecutive: {
      chosen_index = get_consecutive();
      break;
    }
    case kRandomized: {
      chosen_index = get_randomized();
      break;
    }
    case kGreedy: {
      chosen_index = get_greedy();
      break;
    }
    case kCombination: {
      chosen_index = get_combination();
      break;
    }
    default: {
      chosen_index = get_consecutive();
      break;
    }
  }

  workers_.at(chosen_index)->execute(task);
}

void ThreadPool::interrupt() {
  // Restrict further tasks execution
  for (auto worker : workers_)
    worker->suspend();
  // Remove queued tasks
  for (auto worker : workers_)
    worker->interrupt();
}

void ThreadPool::suspend() {
  for (auto worker : workers_)
    worker->suspend();
}

void ThreadPool::start() {
  for (auto worker : workers_)
    worker->start();
}

void ThreadPool::wait() {
  for (auto worker : workers_)
    worker->wait();
}

size_t ThreadPool::size() {
  return workers_.size();
}

int ThreadPool::get_consecutive() {
  ++current_index_; 
  current_index_ %= this->size();
  return current_index_;
}

int ThreadPool::get_randomized() {
  std::default_random_engine r;
  std::uniform_int_distribution<int> rnd(0, this->size() - 1);
  return rnd(r);
}

int ThreadPool::get_greedy() {
  size_t best_index = 0;
  size_t best_size = workers_.at(0)->size();

  for (size_t i = 1, length = this->size(); i < length; ++i) 
    if ( best_size == 0 ) {
      return best_index;
    } else {
      size_t current_size = workers_.at(i)->size();
      if ( current_size < best_size ) {
        best_size = current_size;
        best_index = i;
      }
    }

  return best_index;
}

int ThreadPool::get_combination() {    

  size_t length = this->size();

  if( length == 1 )
    return 0;

  length /= 3;
  if ( length < 2 ) 
    length = 2;

  size_t best_index = 0;
  size_t best_size = workers_.at(0)->size();

  std::default_random_engine r;
  std::uniform_int_distribution<int> rnd(1, this->size() - 1);

  for (size_t i = 0; i < length; ++i) 
    if ( best_size == 0 ) {
      return best_index;
    } else {
      size_t current_index = rnd(r);
      size_t current_size = workers_.at(current_index)->size();
      if ( current_size < best_size ) {
        best_size = current_size;
        best_index = i;
      }
    }
  
  return best_index;
}