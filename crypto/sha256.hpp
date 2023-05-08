//
//  sha256.hpp
//  fart
//
//  Created by Kristian Trenskow on 2023/05/08.
//  Copyright © 2018-2023 Kristian Trenskow. All rights reserved.
//

#ifndef sha256_hpp
#define sha256_hpp

#include "../memory/object.hpp"
#include "../types/data.hpp"

namespace fart::crypto {

	class SHA256 : public Object {

		public:

			SHA256() : _length(0), _bitLength(0), _state({ 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 }) { }

			virtual ~SHA256() { }

			void update(const Data<uint8_t>& data) {

				for (size_t idx = 0 ; idx < data.length() ; idx++) {
					_data[_length] = data[idx];
					_length++;
					if (_length == 64) {
						_transform();
						_bitLength += 512;
						_length = 0;
					}
				}

			}

			Data<uint8_t> finalize() {

				size_t idx = _length;

				if (_length < 56) {
					_data[idx++] = 0x80;
					while (idx < 56) {
						_data[idx++] = 0x00;
					}
				} else {
					_data[idx++] = 0x80;
					while (idx < 64) {
						_data[idx++] = 0x00;
						_transform();
						memset(_data, 0, 56);
					}
				}

				_bitLength += _length * 8;

				_data[63] = _bitLength;
				_data[62] = _bitLength >> 8;
				_data[61] = _bitLength >> 16;
				_data[60] = _bitLength >> 24;
				_data[59] = _bitLength >> 32;
				_data[58] = _bitLength >> 40;
				_data[57] = _bitLength >> 48;
				_data[56] = _bitLength >> 56;

				_transform();

				uint8_t hash[32];

				for (idx = 0; idx < 4; ++idx) {
					hash[idx]      = (_state[0] >> (24 - idx * 8)) & 0x000000ff;
					hash[idx + 4]  = (_state[1] >> (24 - idx * 8)) & 0x000000ff;
					hash[idx + 8]  = (_state[2] >> (24 - idx * 8)) & 0x000000ff;
					hash[idx + 12] = (_state[3] >> (24 - idx * 8)) & 0x000000ff;
					hash[idx + 16] = (_state[4] >> (24 - idx * 8)) & 0x000000ff;
					hash[idx + 20] = (_state[5] >> (24 - idx * 8)) & 0x000000ff;
					hash[idx + 24] = (_state[6] >> (24 - idx * 8)) & 0x000000ff;
					hash[idx + 28] = (_state[7] >> (24 - idx * 8)) & 0x000000ff;
				}

				return Data<uint8_t>(hash, 32);

			}

		private:

			static constexpr uint32_t k[64] = {
				0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
				0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
				0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
				0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
				0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
				0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
				0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
				0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
			};

			template<typename T>
			inline T _rotateRight(T a, T b) const {
				return ((a) >> (b)) | ((a) << (32-(b)));
			}

			template<typename T>
			inline T _ch(T x, T y, T z) const {
				return ((x) & (y)) ^ (~(x) & (z));
			}

			template<typename T>
			inline T _maj(T x, T y, T z) const {
				return ((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z));
			}

			template<typename T>
			inline T _ep0(T x) const {
				return _rotateRight<T>(x,2) ^ _rotateRight<T>(x,13) ^ _rotateRight<T>(x,22);
			}

			template<typename T>
			inline T _ep1(T x) const {
				return _rotateRight<T>(x,6) ^ _rotateRight<T>(x,11) ^ _rotateRight<T>(x,25);
			}

			template<typename T>
			inline T _sig0(T x) const {
				return _rotateRight<T>(x,7) ^ _rotateRight<T>(x,18) ^ ((x) >> 3);
			}

			template<typename T>
			inline T _sig1(T x) const {
				return _rotateRight<T>(x,17) ^ _rotateRight<T>(x,19) ^ ((x) >> 10);
			}

			void _transform() {

				uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

				for (i = 0, j = 0 ; i < 16 ; i++, j += 4) {
					m[i] = (_data[j] << 24) | (_data[j + 1] << 16) | (_data[j + 2] << 8) | (_data[j + 3]);
				}

				for ( ; i < 64 ; i++) {
					m[i] = _sig1(m[i - 2]) + m[i - 7] + _sig0(m[i - 15]) + m[i - 16];
				}

				a = _state[0];
				b = _state[1];
				c = _state[2];
				d = _state[3];
				e = _state[4];
				f = _state[5];
				g = _state[6];
				h = _state[7];

				for (i = 0; i < 64; ++i) {
					t1 = h + _ep1(e) + _ch(e,f,g) + k[i] + m[i];
					t2 = _ep0(a) + _maj(a,b,c);
					h = g;
					g = f;
					f = e;
					e = d + t1;
					d = c;
					c = b;
					b = a;
					a = t1 + t2;
				}

				_state[0] += a;
				_state[1] += b;
				_state[2] += c;
				_state[3] += d;
				_state[4] += e;
				_state[5] += f;
				_state[6] += g;
				_state[7] += h;

			}

			uint8_t _data[64];
			size_t _length;
			uint64_t _bitLength;
			uint32_t _state[8];

	};

}

#endif /* sha256_hpp */
