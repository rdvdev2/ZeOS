#ifndef LIB_PTHREAD_H
#define LIB_PTHREAD_H

void pthread_wrapper(void * (*func)(void*), void* param);

#endif
