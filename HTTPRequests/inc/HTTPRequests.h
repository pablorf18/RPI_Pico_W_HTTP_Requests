#ifndef RPI_PICO_W_HTTP_REQUESTS
#define RPI_PICO_W_HTTP_REQUESTS

/**
 * @file HTTPRequests.h
 * @brief Class to implement the IHTTPRequests interface. Allow the user to send messages through HTTP requests.
          Designed to use with the pico_cyw43_arch_lwip_poll library
**/

#include "IHTTPRequests.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/tcp.h"
#include "lwip/dns.h"

class HTTPRequests : public IHTTPRequests
{
public:

    //constructor
    HTTPRequests() 
    {
        ip_addr_t dns_server;
        IP4_ADDR(&dns_server, 8,8,8,8);
        dns_setserver(0, &dns_server);
    }

    // destructor
    ~HTTPRequests() {}

    /**
     * IHTTPRequests methods
    **/
    std::pair<err_t, ServerResponse> sendRequestToIp(const ip_addr_t & ip, uint32_t port, const std::string & request) override;
    std::pair<err_t, ServerResponse> sendRequestToUrl(const std::string & url, uint32_t port, const std::string & request) override;
    /**
     *
    **/

private:

    //connection timeout is 10s
    uint32_t connectionTimeout_ = 30000;

    /**
     * @brief wait until the request is completed or timeout is reached
     * @param requestData the Request class to get result and response from callbacks
    **/
    err_t waitForRequest(RequestData & requestData);

    /**
     * @brief callback to get the ip once the dns has resolved the url
    **/
    static void dnsFoundCallback(const char *url, const ip_addr_t *ipaddr, void *callback_arg);

    /**
     * @brief init the connection with the ip to send the request data
    **/
    static void connect(const ip_addr_t *ipaddr, RequestData * requestData);

    /**
     * @brief callback to receive the tcp connection
    **/
    static err_t tcpClientConnected(void *arg, struct tcp_pcb *tpcb, err_t err);

    /**
     * @brief callback to get when the server has received the request
    **/
    static err_t tcpClientReceived(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);

    /**
     * @brief callback called when the request failed and server does not receive it
    **/
    static void tcpClientError(void *arg, err_t err);

    /**
     * @brief close the tcp connection
    **/
    static void closeConnection(struct tcp_pcb *tpcb);
};

#endif // RPI_PICO_W_HTTP_REQUESTS