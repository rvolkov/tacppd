// (c)Copyright 2007 by Roman Volkov, Technosev A/S
//
// all comments which start from /**<CR> this is comment for KDoc */
//  for classes
//    @short [short description of the class]
//    @author [class author]
//    @version [class version]
//    @see [references to other classes or methods]
//  for methods
//    @see [references]
//    @return [sentence describing the return value]
//    @exception [list the exeptions that could be thrown]
//    @param [name id] [description]    - can be multiple

#ifndef MYQUEUE_H_
#define MYQUEUE_H_

#include <list>

namespace my_queue_v1 {

template <class T>
class myQueue {
  std::list<T*> q_;
public:
  /**
  @short add to queue */
  void add(T*);
  /**
  @short get from queue */
  T *get();
  /**
  @short empty */
  bool empty();
};



template <class T>
void myQueue<T>::add(T *buf) {
  q_.push_back(buf);
}

template <class T>
T* myQueue<T>::get() {
  T *ret;
  if(q_.empty()) return 0;
  //std::list<T*>::iterator i = q_.begin();
  //ret = *i;
  //i = q_.erase(i);
  ret = *(q_.begin());
  q_.erase(q_.begin());
  return ret;
}

template <class T>
bool
myQueue<T>::empty() {
  return q_.empty();
}


};

namespace queue = my_queue_v1;

#endif /*MYQUEUE_H_*/
