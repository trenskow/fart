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
using namespace fart::serialization;

int main(int argc, const char * argv[]) {
    
    Weak<Array<Type>> weakArray;
    
    try {
        
        auto date = Date("2020-04-06T02:41:20Z");
        
        date.toISO8601()->withCString([](const char* date){
            printf("%s\n", date);
        });
        
    } catch (memory::AllocationException exception) {
        printf("%s (%zu bytes)\n", exception.getDescription(), exception.getSize());
    } catch (types::DecoderException exception) {
        printf("%s (character: %zu)\n", exception.getDescription(), exception.getCharacterIndex());
    } catch (types::OutOfBoundException exception) {
        printf("%s (index: %zu)\n", exception.getDescription(), exception.getIndex());
    } catch (types::KeyNotFoundException<String> exception) {
        exception.getKey().withCString([&exception](const char* key){
            printf("%s (key: %s)\n", exception.getDescription(), key);
        });
    } catch (serialization::JSONMalformedException exception) {
        printf("JSON is malformed (line: %zu, character: %zu)\n", exception.getLine(), exception.getCharacter());
    }
    
    return 0;
    
}
