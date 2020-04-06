//
//  mutex.hpp
//  fart
//
//  Created by Kristian Trenskow on 17/08/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef mutex_hpp
#define mutex_hpp

#include <pthread.h>

namespace fart::threading {
    
    class Semaphore;
    
    class Mutex {
        
    private:
        
        friend class Semaphore;
        
        pthread_mutexattr_t _attr;
        mutable pthread_mutex_t _mutex;
        
    public:
        Mutex() {
            pthread_mutexattr_init(&_attr);
            pthread_mutexattr_settype(&_attr, PTHREAD_MUTEX_RECURSIVE);
            pthread_mutex_init(&_mutex, &_attr);
        }
        
        ~Mutex() {
            pthread_mutex_destroy(&_mutex);
            pthread_mutexattr_destroy(&_attr);
        }
        
        void lock() const {
            pthread_mutex_lock(&_mutex);
        }
        
        void unlock() const {
            pthread_mutex_unlock(&_mutex);
        }
        
        template<typename Func>
        void locked(Func f) const {
            lock();
            f();
            unlock();
        }

        template<typename Func>
        auto lockedValue(Func f) const {
            lock();
            auto ret = f();
            unlock();
            return ret;
        }
    };

}

#endif /* mutex_hpp */
