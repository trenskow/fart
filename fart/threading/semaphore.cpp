//
//  semaphore.cpp
//  fart
//
//  Created by Kristian Trenskow on 14/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#include "./semaphore.hpp"

using namespace fart::threading;

Semaphore::Semaphore() {
    pthread_cond_init(&_condition, nullptr);
}

Semaphore::~Semaphore() {
    pthread_cond_destroy(&_condition);
}

void Semaphore::wait(const Mutex& mutex) const {
    auto err = pthread_cond_wait(&_condition, &mutex._mutex);
}

void Semaphore::signal() const {
    pthread_cond_signal(&_condition);
}

void Semaphore::broadcast() const {
    pthread_cond_broadcast(&_condition);
}
