//
//  main.cpp
//  fart
//
//  Created by Kristian Trenskow on 17/08/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#include <stdio.h>

#include "fart.hpp"

using namespace fart::memory;
using namespace fart::types;
using namespace fart::exceptions;
using namespace fart::network;
using namespace fart::network::web::http;

int main(int argc, const char * argv[]) {
    
    try {
                
        HTTPServer server(3001, [](const HTTPRequest& request, HTTPResponse& response) {
            String responseString = "<html><body><h1>Hello, ";
            responseString.append(request.getPath());
            responseString.append("!</h1></body></html>");
            String contentType("text/html");
            response.setHeaderValue("Content-Type", contentType);
            response.setBody(responseString.getData());
        });
                
        Mutex mtx;
        mtx.lock();
        Semaphore().wait(mtx);
        
    } catch (memory::AllocationException exception) {
        printf("%s (%zu bytes)\n", exception.getDescription(), exception.getSize());
    } catch (types::DecoderException exception) {
        printf("%s (character: %zu)\n", exception.getDescription(), exception.getCharacterIndex());
    } catch (types::OutOfBoundException exception) {
        printf("%s (index: %zu)\n", exception.getDescription(), exception.getIndex());
    } catch (types::KeyNotFoundException<String> exception) {
        printf("%s (key: %s)\n", exception.getDescription(), (const char*)exception.getKey());
    }
    
    return 0;
    
}
