//
// http.hpp
// fart
//
// Created by Kristian Trenskow on 2019/12/12.
// See license in LICENSE.
//

#ifndef http_hpp
#define http_hpp

#include "./head.hpp"
#include "./request-head.hpp"
#include "./response-head.hpp"
#include "../server.hpp"

namespace fart::web::http {

	class HTTPServer : public Server<RequestHead, ResponseHead> {

	public:
		HTTPServer(uint16_t port, function<void(const HTTPRequest& request, HTTPResponse& response)> requestHandler) : Server(port, requestHandler) {}

	protected:

		virtual void postProcess(const Message<RequestHead>& request, Socket &socket) const {
			if (!request.hasHeader("connection") || request.headerValue("connection") != String("keep-alive")) {
				socket.close();
			}
		}

	};

}

#endif /* http_hpp */
