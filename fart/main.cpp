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
using namespace fart::io::fs;
using namespace fart::io::sockets;
using namespace fart::web::http;
using namespace fart::serialization;

int main(int argc, const char * argv[]) {
    
    Weak<Array<Type>> weakArray;
    
    try {
        
        
    } catch (memory::AllocationException exception) {
        printf("%s (%zu bytes)\n", exception.description(), exception.size());
    } catch (types::DecoderException exception) {
        printf("%s (character: %zu)\n", exception.description(), exception.characterIndex());
    } catch (types::OutOfBoundException exception) {
        printf("%s (index: %zu)\n", exception.description(), exception.index());
    } catch (types::KeyNotFoundException<String> exception) {
        exception.key().withCString([&exception](const char* key){
            printf("%s (key: %s)\n", exception.description(), key);
        });
    } catch (serialization::JSONMalformedException exception) {
        printf("JSON is malformed (line: %zu, character: %zu)\n", exception.line(), exception.character());
    }
    
    return 0;
    
}
