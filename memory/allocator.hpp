//
//  allocator.hpp
//  fart
//
//  Created by Kristian Trenskow on 19/07/2021.
//  Copyright © 2021 Kristian Trenskow. All rights reserved.
//

#ifndef allocator_hpp
#define allocator_hpp

#include <math.h>

#include "../exceptions/exception.hpp"
#include "../tools/math.hpp"

#ifndef FART_BLOCK_SIZE
#define FART_BLOCK_SIZE 32
#endif

using namespace fart::tools;
using namespace fart::exceptions::memory;

namespace fart::memory {

	class Allocator {

	public:

		inline void *operator new(size_t size) noexcept(false) {
			return allocate(size);
		}

		inline void operator delete(void *ptr) throw() {
			deallocate(ptr);
		}

	protected:

		inline static uint64_t calculateBufferLength(const uint64_t& minimumLength) {
			// Dynamic heap allocated memory is doubled every time buffer is too small.
			return (uint64_t)pow(2, ceil(log2((math::max<double>(minimumLength, FART_BLOCK_SIZE) / FART_BLOCK_SIZE)))) * FART_BLOCK_SIZE;;
		}

		static void* allocate(size_t size) noexcept(false) {
			void *mem = calloc(size, sizeof(uint8_t));
			if (!mem) throw AllocationException(size);
			return mem;
		}

		inline static void deallocate(void* ptr) throw() {
			free(ptr);
		}

	};

	class NoAllocator {

	public:

		void *operator new(size_t size) = delete;
		void operator delete(void *ptr) throw() = delete;

	};

}

#endif /* allocator_hpp */
