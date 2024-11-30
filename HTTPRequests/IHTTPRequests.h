#ifndef RPI_PICO_W_I_HTTP_REQUESTS
#define RPI_PICO_W_I_HTTP_REQUESTS

/**
 * @file IHTTPRequests.h
 * @brief Interface to send HTTP requests through RPI Pico W
**/

#include <string>
#include "RequestData.h"
#include "lwip/ip_addr.h"

class IHTTPRequests
{
public:

    virtual ~IHTTPRequests() = default;

    /**
     * @brief sendRequestToIp send a request to the given ip and port
     * @param ip the server ip
     * @param port the server port
     * @param request the request to send
     * @return a pair of result and the structure which contains the server response. If err_t is ERR_OK, then the ServerResponse is valid. If err_t is
     *         not ERR_OK, the ServerResponse could contain wrong data
    **/
    virtual std::pair<err_t, ServerResponse> sendRequestToIp(const ip_addr_t & ip, uint32_t port, const std::string & request) = 0;

    /**
     * @brief sendRequestToUrl send a request to the given url and port
     * @param url the server url
     * @param port the server port
     * @param request the request to send
     * @return a pair of result and the structure which contains the server response. If err_t is ERR_OK, then the ServerResponse is valid. If err_t is
     *         not ERR_OK, the ServerResponse could contain wrong data
    **/
    virtual std::pair<err_t, ServerResponse> sendRequestToUrl(const std::string & url, uint32_t port, const std::string & request) = 0;

};

#endif // RPI_PICO_W_I_HTTP_REQUESTS