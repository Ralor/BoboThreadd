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

#include "../include/worker.h"

using namespace BoboThreadd;

Worker::Worker() 
  : canceled_(false), 
    suspended_(true), 
    working_(false), 
    mutex_(new std::mutex()),
    global_mutex_(new std::mutex()),
    tasks_(new std::queue<Task*>())
{					
  std::thread(&Worker::working_function, this).detach();
  mutex_->lock();
  mutex_->unlock();
}

Worker::~Worker() {		
  canceled_ = true;
  global_mutex_->lock();
  global_mutex_->unlock();
  delete mutex_;	
  delete global_mutex_;
  delete tasks_;
}

void Worker::execute(Task* task) {
  mutex_->lock();
  if( !canceled_ )
    tasks_->push(task);
  mutex_->unlock();
}

void Worker::interrupt() {
  mutex_->lock();
  while( !tasks_->empty() ) 
    tasks_->pop();		
  mutex_->unlock();
}

void Worker::wait() {
  do {    
    mutex_->lock();    
    if( !tasks_->empty() || working_ ) {
      mutex_->unlock();
      std::this_thread::sleep_for( std::chrono::milliseconds(17) );
    } else {
      mutex_->unlock();
      break;						
    }
  }	while ( true );
}

void Worker::start() {
  suspended_ = false;
}

void Worker::suspend() {
  suspended_ = true;
}

size_t Worker::size() {
  size_t res;

  mutex_->lock();
  res = tasks_->size();
  mutex_->unlock();

  return res;
}

void Worker::working_function() {
  global_mutex_->lock();

  do {
    if( suspended_ ) {
      std::this_thread::sleep_for( std::chrono::milliseconds(10));
    } else {
      mutex_->lock();
      if( !tasks_->empty() ) {		

        Task* current_task = tasks_->front();
        tasks_->pop();  // pointer Task* cannot be destroyed by pop()
        mutex_->unlock();

        // work() doesn't require synchronization
        working_ = true;
        current_task->work();
        working_ = false;
      } else {
        mutex_->unlock();
        std::this_thread::sleep_for( std::chrono::milliseconds(10));
      }		
    }
  } while( !canceled_ );

  global_mutex_->unlock();
}