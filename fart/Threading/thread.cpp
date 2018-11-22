//
//  thread.cpp
//  fart
//
//  Created by Kristian Trenskow on 22/11/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#include "thread.hpp"

using namespace fart::threading;

Thread::Thread() : _isDetached(false) {}

Thread::~Thread() {
    join();
}

void Thread::_start() {
    _mutex.locked([this]() {
        _isDetached = true;
    });
    _startCallback();
    _mutex.locked([this]() {
        _isDetached = false;
        _startCallback = nullptr;
    });
}

void Thread::detach(function<void ()> startCallback) {
    _mutex.locked([this,startCallback]() {
        
        if (_isDetached) {
            // Handle error;
            return;
        }
        
        _startCallback = startCallback;
        
        pthread_create(&_thread, nullptr, [](void* ctx) {
            ((Thread *)ctx)->_start();
            pthread_exit(nullptr);
            return (void *)0;
        }, this);

    });
}

void Thread::join() const {
    pthread_t thread = _mutex.lockedValue([this](){ return _thread; });
    pthread_join(thread, nullptr);
}

const bool Thread::getIsDetached() const {
    return _mutex.lockedValue([this]() {
        return _isDetached;
    });
}
