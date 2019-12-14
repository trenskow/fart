//
//  head.hpp
//  fart
//
//  Created by Kristian Trenskow on 14/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#ifndef head_hpp
#define head_hpp

#include "../../../memory/object.hpp"
#include "../../../memory/strong.hpp"
#include "../../../types/data.hpp"
#include "../../../types/string.hpp"
#include "../../../types/array.hpp"
#include "./version.hpp"
#include "../message.hpp"

using namespace fart::memory;
using namespace fart::types;

namespace fart::network::web::http {
    
    class Head : public MessageHead {
        
    public:
        
        Head();
        Head(Data<uint8_t>& data);
        Head(const Head& other);
        virtual ~Head();
        
    protected:
        
        static const Version parseVersion(String& version);
        static Strong<Data<uint8_t>> versionData(Version version);
        
        Strong<Array<String>> _parts;
        
    };
    
}

#endif /* head_hpp */
