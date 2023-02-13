//
//  semaphore.hpp
//  fart
//
//  Created by Kristian Trenskow on 2019/12/14.
//  Copyright © 2018-2023 Kristian Trenskow. All rights reserved.
//

#ifndef semaphore_hpp
#define semaphore_hpp

#include <pthread.h>
#include "mutex.hpp"

namespace fart::threading {

	class Semaphore {

	public:
		Semaphore() {
			pthread_cond_init(&_condition, nullptr);
		}

		~Semaphore() {
			pthread_cond_destroy(&_condition);
		}

		void wait(const Mutex& mutex) const {
			pthread_cond_wait(&_condition, &mutex._mutex);
		}

		void signal() const {
			pthread_cond_signal(&_condition);
		}

		void broadcast() const {
			pthread_cond_broadcast(&_condition);
		}

	private:

		mutable pthread_cond_t _condition;

	};

}

#endif /* semaphore_hpp */
