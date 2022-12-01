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
#include <functional>
#include "./mutex.hpp"

using namespace std;

namespace fart::threading {

	class Thread {

	private:

		function<void()> _startCallback;

		pthread_t _thread;
		bool _isDetached;
		Mutex _mutex;

		void _start() {
			_mutex.locked([this]() {
				_isDetached = true;
			});
			_startCallback();
			_mutex.locked([this]() {
				_isDetached = false;
				_startCallback = nullptr;
			});
		}

	public:
		Thread() : _isDetached(false) {};

		~Thread() {
			join();
		}

		void detach(function<void()> startCallback) {
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

		void join() const {
			pthread_t thread = _mutex.lockedValue([this](){ return _thread; });
			pthread_join(thread, nullptr);
		}

		bool isDetached() const {
			return _mutex.lockedValue([this]() {
				return _isDetached;
			});
		}

	};

}

#endif /* thread_hpp */
