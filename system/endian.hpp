//
//  endian.hpp
//  fart
//
//  Created by Kristian Trenskow on 2020/04/01.
//  Copyright © 2018-2023 Kristian Trenskow. All rights reserved.
//

#ifndef endian_hpp
#define endian_hpp

#include <stdint.h>

namespace fart::system {

	class Endian {

	private:
		template<typename T>
		static const T swap(T val) {
			T ret;
			uint8_t* valP = (uint8_t *)&val;
			uint8_t* retP = (uint8_t *)&ret;
			for (size_t idx = 0 ; idx < sizeof(T) ; idx++) {
				retP[idx] = valP[sizeof(T) - idx - 1];
			}
			return ret;
		}

	public:

		enum class Variant {
			little,
			big
		};

		inline static Variant systemVariant() {
			static const uint32_t test = 0x00000001;
			return *((uint8_t *)&test) == 0x01 ? Variant::little : Variant::big;
		}

		template<typename T>
		inline static const T convert(T val, Variant from, Variant to) {
			if (from == to) return val;
			return swap(val);
		}

		template<typename T>
		static const T toSystemVariant(T val, Variant from) {
			return convert(val, from, systemVariant());
		}

		template<typename T>
		static const T fromSystemVariant(T val, Variant to) {
			return convert(val, systemVariant(), to);
		}

	};

}

#endif /* endian_hpp */
