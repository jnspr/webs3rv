#include "application.hpp"
#include "config_parser.hpp"
#include "config_tokenizer.hpp"

#include <iostream>


struct uploadData{
    Slice boundary;
    Slice contentDisposition;
    Slice name;
    Slice filename;
    Slice contentType;
    Slice fileContent;
    ssize_t fileSize;

    uploadData()
    {
        this->fileSize = 0;
    }
}    ;

void parseupload(std::vector<uint8_t> body, ssize_t contentLength, uploadData &data)
{
    // body.data is used for getting char * from vector maybe need to cast this to char *

    // need to add -- to beginning of boundary for start and -- to the end to end
    // all values from header are in request vector headers (not finished yet)
    (void)contentLength;
    // gets the filesize from the http header via strnstr
    printf("Parsing upload\n");
    printf("Contentlength: %ld\n", contentLength);

    // reads the upload body (file) into a new buffer


    // creates a slice from the bodybuffer
     Slice sliceBod((char *) body.data(), body.size());
   // sliceBuf.splitStart('\n', data.boundary);


   // std::cout << "Boundary: \n" << data.boundary << std::endl;
    std::cout << "slicebuf after first slice: " << sliceBod << std::endl;
   (void) data;
}

int main(int argc, char *argv[])
{


}
