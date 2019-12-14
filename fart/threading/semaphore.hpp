//
//  semaphore.hpp
//  fart
//
//  Created by Kristian Trenskow on 14/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#ifndef semaphore_hpp
#define semaphore_hpp

#include <pthread.h>
#include "mutex.hpp"

namespace fart::threading {

    class Semaphore {
        
    public:
        Semaphore();
        ~Semaphore();
        
        void wait(const Mutex& mutex) const;
        void signal() const;
        void broadcast() const;
        
    private:
        
        mutable pthread_cond_t _condition;
        
    };
    
}

#endif /* semaphore_hpp */
