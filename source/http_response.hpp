#ifndef HTTP_RESPONSE_hpp
#define HTTP_RESPONSE_hpp

#include <string>
#include <sstream>

#include "slice.hpp"

class HttpResponse
{
public:
    HttpResponse();

    void initialize(int statusCode, Slice statusMessage, const void *bodyBuffer, size_t bodySize);
    void initialize(int statusCode, Slice statusMessage, int bodyFileno, size_t bodySize);
    void addHeader(Slice key, Slice value);

    bool hasData();
    void transferToSocket(int fileno);
private:
    std::stringstream _headerStream;
    std::string       _headerString;
    size_t            _headerRemainder;
    int               _bodyFileno;
    const void       *_bodyBuffer;
    size_t            _bodyRemainder;
};

#endif // HTTP_RESPONSE_hpp
