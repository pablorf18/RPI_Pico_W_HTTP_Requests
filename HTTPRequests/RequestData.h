#ifndef RPI_PICO_W_REQUEST_DATA
#define RPI_PICO_W_REQUEST_DATA

/**
 * @file RequestData.h
 * @brief Class to define the data needed to send an HTTP response
**/

#include <string>
#include <cstdint>
#include "lwip/err.h"

enum class RequestType
{
    GET,
    POST,
    PUT,
    DELETE
};

struct ServerResponse
{
    std::string payload;
    err_t err;
    bool requestComplete = false;
};

/**
 * RequestData allows the client to send the required data through a HTTP request. Is a templatized class
 * to allow send data to an ip address or to an url server. Also, the kind of data is customizable
**/
class RequestData
{
public:
    RequestData(uint32_t port, const std::string & request, ServerResponse & serverResponse)
        : port_(port), request_(request), serverResponse_(serverResponse)
    {
    }

    ~RequestData() {}

    /**
    * Getter methods to access information 
    **/
    uint32_t getPort() const { return port_; }
    std::string getRequest() const { return request_; }
    
    /**
    * getServerResponse: get the reference of the server response to fill the data
    **/
    ServerResponse & getServerResponse() { return serverResponse_; }

private:

    // the server port
    uint32_t port_;
    // the request
    std::string request_;

    //reference server response to be filled by the callbacks
    ServerResponse & serverResponse_;
};

#endif