//
//  response.hpp
//  fart
//
//  Created by Kristian Trenskow on 12/12/2019.
//  Copyright © 2019 Kristian Trenskow. All rights reserved.
//

#ifndef response_hpp
#define response_hpp

#include "./head.hpp"
#include "../message.hpp"

namespace fart::network::web::http {
    
    class ResponseHead : public Head {
        
    public:
        
        enum Status {
            OK = 200
        };
        
        ResponseHead();
        ResponseHead(Data<uint8_t>& data);
        ResponseHead(const ResponseHead& other);
        virtual ~ResponseHead();
        
        const Version version() const;
        const Status status() const;
        
    protected:
        
        virtual Strong<Data<uint8_t>> headData(const Data<uint8_t>& lineBreak) const;
        
    private:
        
        Version _version;
        Status _status;
        
    };
    
    typedef Message<ResponseHead> HTTPResponse;
    
}

#endif /* response_hpp */
