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
        
        auto dict = Dictionary<String, Date>();
        
        dict.set("Test", Strong<Date>("2020-04-04T09:07:50Z"));
                
        printf("%s\n", JSON::stringify(dict.as<Type>())->getCString());
                
    } catch (memory::AllocationException exception) {
        printf("%s (%zu bytes)\n", exception.getDescription(), exception.getSize());
    } catch (types::DecoderException exception) {
        printf("%s (character: %zu)\n", exception.getDescription(), exception.getCharacterIndex());
    } catch (types::OutOfBoundException exception) {
        printf("%s (index: %zu)\n", exception.getDescription(), exception.getIndex());
    } catch (types::KeyNotFoundException<String> exception) {
        printf("%s (key: %s)\n", exception.getDescription(), (const char*)exception.getKey());
    } catch (serialization::JSONMalformedException exception) {
        printf("JSON is malformed (line: %zu, character: %zu)\n", exception.getLine(), exception.getCharacter());
    }
    
    return 0;
    
}
