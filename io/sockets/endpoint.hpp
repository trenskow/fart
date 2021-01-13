//
//  endpoint.hpp
//  fart
//
//  Created by Kristian Trenskow on 19/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef endpoint_hpp
#define endpoint_hpp

#include <sys/socket.h>

#include "../../memory/object.hpp"
#include "../../types/string.hpp"
#include "../../threading/mutex.hpp"

using namespace fart::memory;
using namespace fart::types;
using namespace fart::threading;

namespace fart::io::sockets {

	enum class EndpointType {
		IPv4 = 0,
		IPv6
	};

	class Endpoint : public Object {

	private:

		sockaddr_storage _storage;
		Mutex _mutex;

	public:

		Endpoint(Strong<String> host, uint16_t port, EndpointType type = EndpointType::IPv4, uint32_t scope_id = 0) {

			switch (type) {
				case EndpointType::IPv4: {

					sockaddr_in* addr = (sockaddr_in *)&this->_storage;

					addr->sin_len = sizeof(sockaddr_in);
					addr->sin_family = AF_INET;
					addr->sin_addr.s_addr = INADDR_ANY;
					addr->sin_port = htons(port);

					host->withCString([&addr](const char* host){
						inet_pton(AF_INET, host, &addr->sin_addr);
					});

				} break;
				case EndpointType::IPv6: {

					sockaddr_in6* addr = ((sockaddr_in6*)&this->_storage);

					addr->sin6_len = sizeof(sockaddr_in6);
					addr->sin6_family = AF_INET6;
					addr->sin6_addr = in6addr_any;
					addr->sin6_port = htons(port);
					addr->sin6_scope_id = scope_id;

					host->withCString([&addr](const char* host) {
						inet_pton(AF_INET6, host, &addr->sin6_addr);
					});

				} break;
			}

		}

		Endpoint(const char* host, uint16_t port, EndpointType types = EndpointType::IPv4, uint32_t scope_id = 0)  : Endpoint(Strong<String>(host), port, types, scope_id) {}

		Endpoint(sockaddr* addr) {
			memcpy(&this->_storage, addr, addr->sa_len);
		}

		Endpoint(uint16_t port, EndpointType types = EndpointType::IPv4, uint32_t scope_id = 0)  : Endpoint("0.0.0.0", port, types, scope_id) {}

		virtual ~Endpoint() {}

		EndpointType type() const {
			return _mutex.lockedValue([this]() {
				if (this->_storage.ss_family == AF_INET) return EndpointType::IPv4;
				return EndpointType::IPv6;
			});
		}

		Strong<String> host() const {

			return _mutex.lockedValue([this]() {

				char ret[1024];

				switch (this->type()) {
					case EndpointType::IPv4:
						inet_ntop(AF_INET, &((sockaddr_in *)&this->_storage)->sin_addr, ret, 1024);
						break;
					case EndpointType::IPv6:
						inet_ntop(AF_INET6, &((sockaddr_in6 *)&this->_storage)->sin6_addr, ret, 1024);
						break;
				}

				return Strong<String>(ret);

			});

		}

		uint16_t port() const {
			return _mutex.lockedValue([this]() {
				switch (this->type()) {
					case EndpointType::IPv4:
						return ntohs(((sockaddr_in *)&this->_storage)->sin_port);
					case EndpointType::IPv6:
						return ntohs(((sockaddr_in6 *)&this->_storage)->sin6_port);
				}
			});
		}

		void setPort(const uint16_t port) {
			_mutex.locked([this,port]() {
				switch (this->type()) {
					case EndpointType::IPv4:
						((sockaddr_in *)&this->_storage)->sin_port = htons(port);
						break;
					case EndpointType::IPv6:
						((sockaddr_in6 *)&this->_storage)->sin6_port = htons(port);
						break;
				}
			});
		}

		const sockaddr* sockAddr() const {
			return _mutex.lockedValue([this]() {
				return (sockaddr *)&this->_storage;
			});
		}

	};

}

#endif /* endpoint_hpp */
