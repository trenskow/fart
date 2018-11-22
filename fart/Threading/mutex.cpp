//
//  mutex.cpp
//  fart
//
//  Created by Kristian Trenskow on 17/08/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#include "mutex.hpp"

using namespace fart::threading;

Mutex::Mutex() {
    pthread_mutexattr_init(&_attr);
    pthread_mutexattr_settype(&_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&_mutex, &_attr);
}

Mutex::~Mutex() {
    pthread_mutex_destroy(&_mutex);
    pthread_mutexattr_destroy(&_attr);
}

void Mutex::lock() const {
    pthread_mutex_lock(&_mutex);
}

void Mutex::unlock() const {
    pthread_mutex_unlock(&_mutex);
}
