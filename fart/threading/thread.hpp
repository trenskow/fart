//
//  thread.hpp
//  fart
//
//  Created by Kristian Trenskow on 22/11/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef thread_hpp
#define thread_hpp

#include <pthread.h>
#include <thread>
#include "./mutex.hpp"

using namespace std;

namespace fart::threading {
        
    class Thread {
        
    private:
        
        function<void()> _startCallback;
        
        pthread_t _thread;
        bool _isDetached;
        Mutex _mutex;
        
        void _start();
        
    public:
        Thread();
        ~Thread();
        
        void detach(function<void()> startCallback);
        void join() const;
        
        const bool isDetached() const;
        
    };

}

#endif /* thread_hpp */
