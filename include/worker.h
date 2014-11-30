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

#ifndef BBTHREADD_WORKER_H_
#define BBTHREADD_WORKER_H_

#include <queue>
#include <thread>
#include <mutex>
#include <chrono>

#include "task.h"

namespace BoboThreadd {

// Executes tasks in detached thread
// Thread recieving tasks from FIFO
class Worker {

public:

  Worker();  // Starts detached thread that execute given tasks
  ~Worker(); // Stops detached thread
  
  void execute(Task*);       // Add task to worker queue  
  void interrupt();          // Removes all tasks from queue    
  void wait();  // Blocks calling thread until all tasks will be executed
  void start();              // Allows tasks execution  
  void suspend();            // Restricts tasks execution  
  size_t size();             // Returns tasks_ size  

private:

  void working_function();

  // canceled_ is "true" when Worker should be turned off
  bool				canceled_;
  // suspended_ is "true" when Worker should sleep(10)
  bool				suspended_;
  // working_ is "true" when Worker executing some task
  bool				working_;
  std::queue<Task*> *tasks_;	
  // Critical section needed to control thread-unsafe std::queue
  std::mutex		*mutex_;
  // We need one more mutex to be sure that thread is stopped
  std::mutex        *global_mutex_;
};

} // namespace BoboThreadd

#endif // BBTHREADD_WORKER_H_
