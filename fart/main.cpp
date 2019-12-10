//
//  main.cpp
//  fart
//
//  Created by Kristian Trenskow on 17/08/2018.
//  Copyright © 2018 Kristian Trenskow. All rights reserved.
//

#include <stdio.h>

#include "strong.hpp"
#include "exception.hpp"
#include "string.hpp"
#include "array.hpp"
#include "weak.hpp"
#include "number.hpp"
#include "dictionary.hpp"
#include "socket.hpp"
#include "endpoint.hpp"

using namespace fart::memory;
using namespace fart::types;
using namespace fart::exceptions;
using namespace fart::network;

int main(int argc, const char * argv[]) {
    
    try {
        
        Array<String> array;
        
        Dictionary<String, Number<double>> dictionary;
        
        Number<double> doubleValue = 12.0;
        Strong<Number<uint64_t>> intValue(doubleValue);
        
        dictionary.set("testKey", Strong<Number<double>>(12));
        
        printf("Stored value is %1.0f.\n", (double)*dictionary.get("testKey"));
        
        String string = "This is my string";
        
        array.append(string); // String copied to heap
        
        Weak<String> weak;
        
        Strong<String> otherString = array[0];
        
        array[0]->append(" - and I'm testing it!");
        
        array.removeItemAtIndex(0);
        
        printf("%s\n", otherString->getCString());
        
        otherString->split(" ")->forEach([](Strong<String> str) {
            printf("%s\n", str->getCString());
        });
        
        printf("%s\n", String::join(otherString->split(" "), " ")->getCString());
        
        Strong<Socket> socket;
        
        Strong<Array<Socket>> openSockets;
        
        socket->bind(Strong<Endpoint>(Strong<String>("0.0.0.0"), 3004));
        socket->listen([&](Strong<Socket> newSocket) {
            openSockets->append(newSocket);
            newSocket->accept([](Strong<Data<uint8_t>> data, Strong<Endpoint> endpoint) {
                Strong<String> string(data);
                printf("%s", string->getCString());
            });
        });
        
        usleep(10000000);
        
        bool done = false;
        
        do {
            usleep(100000);
            done = true;
            for (size_t idx = 0 ; idx < openSockets->getCount() ; idx++) {
                if (openSockets->getItemAtIndex(idx)->getSocketState() != SocketStateClosed) {
                    done = false;
                    break;
                }
            }
        } while(!done);
            
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
