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

namespace fart {
    namespace threading {
        
        class Mutex {
            
        private:
            pthread_mutexattr_t _attr;
            mutable pthread_mutex_t _mutex;
            
        public:
            Mutex();
            ~Mutex();
            
            void lock() const;
            void unlock() const;
            
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
}

#endif /* mutex_hpp */
