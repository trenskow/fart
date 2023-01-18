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

	public:

		Endpoint(const String& host, uint16_t port, EndpointType type = EndpointType::IPv4, uint32_t scope_id = 0) {

			switch (type) {
				case EndpointType::IPv4: {

					sockaddr_in* addr = (sockaddr_in *)&this->_storage;

#if __APPLE__
					addr->sin_len = sizeof(sockaddr_in);
#endif
					addr->sin_family = AF_INET;
					addr->sin_addr.s_addr = INADDR_ANY;
					addr->sin_port = htons(port);

					host.withCString([&addr](const char* host){
						inet_pton(AF_INET, host, &addr->sin_addr);
					});

				} break;
				case EndpointType::IPv6: {

					sockaddr_in6* addr = ((sockaddr_in6*)&this->_storage);

#if __APPLE__
					addr->sin6_len = sizeof(sockaddr_in6);
#endif
					addr->sin6_family = AF_INET6;
					addr->sin6_addr = in6addr_any;
					addr->sin6_port = htons(port);
					addr->sin6_scope_id = scope_id;

					host.withCString([&addr](const char* host) {
						inet_pton(AF_INET6, host, &addr->sin6_addr);
					});

				} break;
			}

		}

		Endpoint(const sockaddr* addr, size_t length) {
			memcpy(&this->_storage, addr, length);
		}

		Endpoint(uint16_t port, EndpointType types = EndpointType::IPv4, uint32_t scope_id = 0) : Endpoint("0.0.0.0", port, types, scope_id) {}

		virtual ~Endpoint() {}

		EndpointType type() const {
			if (this->_storage.ss_family == AF_INET) return EndpointType::IPv4;
			return EndpointType::IPv6;
		}

		String host() const {

			char ret[1024];

			switch (this->type()) {
				case EndpointType::IPv4:
					inet_ntop(AF_INET, &((sockaddr_in *)&this->_storage)->sin_addr, ret, 1024);
					break;
				case EndpointType::IPv6:
					inet_ntop(AF_INET6, &((sockaddr_in6 *)&this->_storage)->sin6_addr, ret, 1024);
					break;
			}

			return String(ret);

		}

		uint16_t port() const {
			switch (this->type()) {
				case EndpointType::IPv4:
					return ntohs(((sockaddr_in *)&this->_storage)->sin_port);
				case EndpointType::IPv6:
					return ntohs(((sockaddr_in6 *)&this->_storage)->sin6_port);
			}
		}

		void setPort(const uint16_t port) {
			switch (this->type()) {
				case EndpointType::IPv4:
					((sockaddr_in *)&this->_storage)->sin_port = htons(port);
					break;
				case EndpointType::IPv6:
					((sockaddr_in6 *)&this->_storage)->sin6_port = htons(port);
					break;
			}
		}

		const sockaddr* sockAddr() const {
			return (sockaddr *)&this->_storage;
		}

		const size_t sockAddrLength() const {
			switch (this->type()) {
				case EndpointType::IPv4:
					return sizeof(sockaddr_in);
				case EndpointType::IPv6:
					return sizeof(sockaddr_in6);
			}
			return 0;
		}

	};

}

#endif /* endpoint_hpp */
