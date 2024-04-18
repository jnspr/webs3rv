#ifndef UPLOAD_HANDLER_hpp
#define UPLOAD_HANDLER_hpp

#include "routing.hpp"
#include "http_request.hpp"

namespace UploadHandler
{
    /* Handles the upload of one or multiple files */
    void handleUpload(const HttpRequest &request, const RoutingInfo &routingInfo);
};

#endif // UPLOAD_HANDLER_hpp
