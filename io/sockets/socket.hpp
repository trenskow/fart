//
//  socket.hpp
//  fart
//
//  Created by Kristian Trenskow on 19/09/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#ifndef socket_hpp
#define socket_hpp

#include <arpa/inet.h>

#include "../../memory/object.hpp"
#include "../../memory/strong.hpp"
#include "../../memory/weak.hpp"
#include "../../threading/thread.hpp"
#include "../../threading/mutex.hpp"
#include "../../types/data.hpp"
#include "../../types/data.hpp"
#include "./endpoint.hpp"

#define BUFFER_SIZE 16384

using namespace fart::memory;
using namespace fart::threading;
using namespace std;

namespace fart::io::sockets {

	enum class SocketState {
		closed = 0,
		listening,
		connected
	};

	class Socket : public Object, public Hashable {

		friend class Strong<Socket>;

	public:

		class ICloseListener {
		public:
			virtual void SocketClosed(const Socket& socket) const = 0;
		};

		static Strong<Socket> create(bool isUDP = false) {
			return Strong<Socket>(isUDP);
		}

		virtual ~Socket() {
			close();
			_closeCallback.callback = nullptr;
		}

		virtual uint64_t hash() const override {
			return _mutex.lockedValue([this]() {
				return _socket;
			});
		}

		void bind(const Endpoint& endpoint) {

			_mutex.locked([this,endpoint](){

				_localEndpoint = endpoint;

				const sockaddr* addr = _localEndpoint.sockAddr();

				if (_socket < 0) {

					_socket = socket(addr->sa_family, (_isUDP ? SOCK_DGRAM : SOCK_STREAM), (_isUDP ? IPPROTO_UDP : IPPROTO_TCP));

					if (_socket < 0) {
						// TODO: Handle error
						return;
					}

					if (_localEndpoint.type() == EndpointType::IPv6) {
						int32_t on = 1;
						setsockopt(_socket, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));
					}

				}

				if (!_isUDP) {
					int reuseaddr = 1;
					setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
				}

				if (::bind(_socket, addr, addr->sa_len) != 0) {
					// Handle error
					return;
				}

			});

		}

		void listen(function<void(Socket&)> acceptCallback) {

			this->retain();

			_mutex.locked([this,acceptCallback]() {

				if (_state != SocketState::closed) {
					close();
				}

				_listenThread.detach([this,acceptCallback]() {

					_mutex.locked([this]() {

						if (::listen(_socket, 5) != 0) {
							// Handle error;
							return;
						}

						_state = SocketState::listening;

					});

					int newSocketFd = -1;

					do {

						sockaddr_storage addr;
						socklen_t len;

						int socket = _mutex.lockedValue([this](){ return _socket; });

						newSocketFd = ::accept(socket, (sockaddr *)&addr, &len);

						if (newSocketFd >= 0) {
							Strong<Socket> newSocket(newSocketFd);
							acceptCallback(newSocket);
							if (newSocket->socketState() != SocketState::connected) {
								newSocket->close();
							}
						}

					} while (newSocketFd >= 0);

					close();

					this->release();

				});

			});

		}

		void accept(function<void(const Data<uint8_t>&, const Endpoint&)> readCallback) {
			_mutex.locked([this]() {
				_state = SocketState::connected;
			});
			_read([]() {}, readCallback);
		}

		void connect(const Endpoint& endpoint, function<void(const Data<uint8_t>&, const Endpoint&)> readCallback) {

			_mutex.locked([this,endpoint,readCallback]() {

				if (_isUDP) {
					// Handle error;
					return;
				}

				if (_state != SocketState::closed) {
					close();
				}

				_remoteEndpoint = endpoint;

				_socket = socket(_remoteEndpoint.sockAddr()->sa_family, SOCK_STREAM, IPPROTO_TCP);

				if (_socket < 0) {
					// Handle error;
					return;
				}

			});

			_read([this]() {

				_mutex.locked([this]() {

					if (::connect(_socket, _remoteEndpoint.sockAddr(), _remoteEndpoint.sockAddr()->sa_len) != 0) {
						// Handle error
						return;
					}

					_state = SocketState::connected;

				});

			}, readCallback);

		}

		size_t send(const Data<uint8_t>& data) const {
			return ::send(_socket, data.items(), data.length(), 0);
		}

		size_t sendTo(const Endpoint& endpoint, const Data<uint8_t>& data) const {
			return 0;
		}

		void close() {
			if (_state != SocketState::closed && _closeCallback.callback != nullptr) {
				_closeCallback.callback(*this, _closeCallback.context);
			}
			_mutex.locked([this]() {
				shutdown(_socket, SHUT_RDWR);
				::close(_socket);
				_socket = -1;
				_state = SocketState::closed;
			});
		}

		Endpoint localEndpoint() const {
			return _mutex.lockedValue([this]() {
				return this->_localEndpoint;
			});
		}

		Endpoint remoteEndpoint() const {
			return _mutex.lockedValue([this]() {
				return this->_remoteEndpoint;
			});
		}

		bool isUDP() const {
			return _mutex.lockedValue([this]() {
				return this->_isUDP;
			});
		}

		SocketState socketState() const {
			return _mutex.lockedValue([this]() {
				return this->_state;
			});
		}

		void awaitClose() const {
			if (_receiveThread.isDetached()) _receiveThread.join();
			if (_listenThread.isDetached()) _listenThread.join();
		}

		void setCloseCallback(void (*callback)(const Socket& socket, void* context), void* context) {
			_closeCallback.callback = callback;
			_closeCallback.context = context;
		}

		bool operator==(const Socket& other) const {
			return _socket == other._socket;
		}

	private:

		struct CloseCallback {
			CloseCallback() : callback(nullptr), context(nullptr) {}
			void (*callback)(const Socket& socket, void* context);
			void *context;
		};

		Socket(bool isUDP = false) : _isUDP(isUDP), _socket(-1), _state(SocketState::closed), _localEndpoint(nullptr), _remoteEndpoint(nullptr) {}

		bool _isUDP;

		int _socket;
		SocketState _state;

		Endpoint _localEndpoint;
		Endpoint _remoteEndpoint;

		Thread _listenThread;
		Thread _receiveThread;

		Mutex _mutex;

		CloseCallback _closeCallback;

		void _read(function<void()> setup, function<void(const Data<uint8_t>&, const Endpoint& endpoint)> readCallback) {

			this->retain();

			_receiveThread.detach([this,setup,readCallback]() {

				setup();

				ssize_t bytesRead = 0;
				uint8_t buffer[BUFFER_SIZE];

				do {

					_mutex.lock();

					int socket = _socket;
					Strong<Endpoint> endpoint = nullptr;

					if (!_isUDP) {
						endpoint = _remoteEndpoint;
						_mutex.unlock();
						bytesRead = recv(socket, buffer, BUFFER_SIZE, 0);
					} else {
						_mutex.unlock();
						sockaddr_storage addr;
						socklen_t len = sizeof(sockaddr_storage);
						bytesRead = recvfrom(socket, buffer, BUFFER_SIZE, 0, (sockaddr *)&addr, &len);
						endpoint = Strong<Endpoint>((sockaddr*)&addr);
					}
					if (bytesRead > 0) {
						Data<uint8_t> data(buffer, bytesRead);
						readCallback(data, endpoint);
					} else {
						close();
					}
				} while (bytesRead > 0);

				close();

				this->release();

			});

		}

	};

}

#endif /* socket_hpp */
