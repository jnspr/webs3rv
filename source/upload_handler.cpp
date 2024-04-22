#include "upload_handler.hpp"
#include "http_exception.hpp"

#include <fstream>

/* Attempts to parse the request's `Content-Type` header to obtain the form boundary */
static bool extractBoundary(const HttpRequest &request, Slice &outBoundary)
{
    Slice prefix;

    // Check if the content type header is present
    const HttpRequest::Header *contentTypeHeader = request.findHeader(C_SLICE("Content-Type"));
    if (contentTypeHeader == NULL)
        return false;
    Slice contentType = contentTypeHeader->getValue();

    // Check if the content type is form data
    if (!contentType.splitStart(';', prefix))
        return false;
    if (prefix != C_SLICE("multipart/form-data"))
        return false;

    // Find the boundary parameter
    contentType.stripStart(' ');
    while (contentType.getLength() > 0)
    {
        if (!contentType.splitStart(';', prefix))
        {
            prefix = contentType;
            contentType = Slice();
        }
        if (prefix.consumeStart(C_SLICE("boundary=")))
        {
            outBoundary = prefix;
            return true;
        }
        contentType.stripStart(' ');
    }
    return false;
}

/* Attempts to parse the form field's header to obtain the file name */
static bool extractFileName(Slice header, Slice &outFileName)
{
    Slice line;

    while (header.getLength() > 0)
    {
        // Get the next header line
        if (!header.splitStart(C_SLICE("\r\n"), line))
        {
            line = header;
            header = Slice();
        }

        // Stop parsing if the line is invalid
        Slice key, value = line;
        if (!value.splitStart(C_SLICE(": "), key))
            return false;

        // Ignore the line if it doesn't set the content disposition
        if (key != C_SLICE("Content-Disposition"))
            continue;

        // Stop parsing if the content disposition is not form data
        if (!value.consumeStart(C_SLICE("form-data; ")))
            return false;

        // Parse the form data parameters
        while (value.getLength() > 0)
        {
            // Get the next parameter
            Slice parameterKey, parameterValue;
            if (!value.splitStart(C_SLICE("; "), parameterValue))
            {
                parameterValue = value;
                value = Slice();
            }

            // Ignore the parameter if it doesn't have a value
            if (!parameterValue.splitStart('=', parameterKey))
                continue;

            // Ignore the parameter if it isn't a file name
            if (parameterKey != C_SLICE("filename"))
                continue;

            // The file name has been found, remove the quotes and return successfully
            parameterValue.removeDoubleQuotes();
            outFileName = parameterValue;
            return true;
        }
    }

    return false;
}

/* Handles the content between form boundaries */
static void handleField(Slice field, const RoutingInfo &routingInfo)
{
    Slice header;
    Slice fileName;

    // Ignore fields without a header or without a file name
    if (!field.splitStart(C_SLICE("\r\n\r\n"), header))
        return;
    if (!extractFileName(header, fileName))
        return;

    // Clamp the file name to not go below the upload directory and assemble the full path
    if (!Utility::checkPathLevel(fileName))
        throw HttpException(403);
    std::string path = Slice(routingInfo.nodePath).stripEnd('/').toString() + '/' + fileName.toString();

    // Attempt to write the file
    std::ofstream stream(path.c_str(), std::ios::binary | std::ios::trunc);
    if (!stream.is_open())
        throw HttpException(500);
    stream.write(&field[0], field.getLength());
    if (!stream.good())
        throw HttpException(500);
}

/* Handles the upload of one or multiple files */
void UploadHandler::handleUpload(const HttpRequest &request, const RoutingInfo &routingInfo)
{
    Slice body(request.body);
    Slice field;

    // Extract the form boundary from the request's content type header
    Slice boundarySlice;
    if (!extractBoundary(request, boundarySlice))
        throw HttpException(400);
    std::string boundary = "--" + boundarySlice.toString();

    // Consume the first boundary
    if (body.getLength() == 0)
        return;
    if (!body.consumeStart(boundary))
        throw HttpException(400);
    if (!body.consumeStart(C_SLICE("\r\n")))
        throw HttpException(400);

    // Also consume the CRLF after the first boundary
    boundary = "\r\n" + boundary;

    // Consume boundaries
    while (body.getLength() > 0)
    {
        // Extract and handle the form field up to the next boundary
        if (!body.splitStart(boundary, field))
            throw HttpException(400);
        handleField(field, routingInfo);

        // Break if the final boundary is reached
        if (body.consumeStart(C_SLICE("--")))
            break;
        if (!body.consumeStart(C_SLICE("\r\n")))
            throw HttpException(400);
    }
}
