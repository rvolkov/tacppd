// (c)Copyright 2007 by Roman Volkov, Technosev A/S
//
/*
#include "myQueue.h"

template <class T>
my_queue_v1::myQueue<T>::myQueue() {
//  pthread_mutex_init(&mutex_,0);
}

template <class T>
my_queue_v1::myQueue<T>::~myQueue() {
//  pthread_mutex_destroy(&mutex_);
}

//void
//my_queue_v1::myQueue::lock() {
//  pthread_mutex_lock(&mutex_);
//}

//void
//my_queue_v1::myQueue::unlock() {
//  pthread_mutex_unlock(&mutex_);
//}

template <class T>
void
my_queue_v1::myQueue<T>::add(T *buf) {
//  lock();
  q_.push_back(buf);
//  unlock();
}

template <class T>
T*
my_queue_v1::myQueue<T>::get() {
  T *ret;
  if(q_.empty()) {
    return 0;
  }
  std::list<T*>::iterator i;
  i = q_.begin();
  ret = *i;
  i = q_.erase(i);
  return ret;
}

bool
my_queue_v1::myQueue<T>::empty() {
  return q_.empty();
}
*/
