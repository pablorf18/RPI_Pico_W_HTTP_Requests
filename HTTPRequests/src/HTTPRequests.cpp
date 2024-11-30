#include "HTTPRequests.h"

#include <stdio.h>

std::pair<err_t, ServerResponse> 
    HTTPRequests::sendRequestToIp(const ip_addr_t & ip, uint32_t port, const std::string & request)
{
    std::pair<err_t, ServerResponse> result;

    printf("HTTPRequests::sendRequestToIp: sending request to ip=%s, port=%u\n", 
           ip4addr_ntoa(&ip), port);

    //construct the requestData
    ServerResponse serverResponse;
    RequestData requestData {port, request, serverResponse};

    //start the tcp connection with the ip
    connect(&ip, &requestData);

    result.first = waitForRequest(requestData);
    result.second = serverResponse;

    return result;
}

std::pair<err_t, ServerResponse> 
    HTTPRequests::sendRequestToUrl(const std::string & url, uint32_t port, const std::string & request)
{
    std::pair<err_t, ServerResponse> result;

    //construct the requestData and cast it to void* to add to the callbacks
    ServerResponse serverResponse;
    RequestData requestData {port, request, serverResponse};
    void* callback_arg = (void*)&requestData;

    printf("HTTPRequests::sendRequestToUrl: Resolving DNS for %s\n", url.c_str());
    ip_addr_t serverIP;
    err_t dns_err = dns_gethostbyname(url.c_str(), &serverIP, dnsFoundCallback, callback_arg);
    if (dns_err == ERR_OK) 
    {
        // DNS is already in the local table, so we can proceed immediately
        connect(&serverIP, &requestData);
    }
    else if(dns_err != ERR_INPROGRESS) 
    {
        printf("HTTPRequests::sendRequestToUrl: DNS resolution failed immediately: %d\n", dns_err);
        result.first = dns_err;
        return result;
    }

    result.first = waitForRequest(requestData);
    result.second = serverResponse;

    return result;
}


/////////////////////
// PRIVATE METHODS //
/////////////////////

err_t HTTPRequests::waitForRequest(RequestData & requestData)
{
    err_t result = ERR_OK;

    //loop to wait for server response or timeout
    bool timeoutReached = false;
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    printf("HTTPRequests::waitForRequest: waiting for server response\n");
    while (!timeoutReached) 
    {
        if(requestData.getServerResponse().requestComplete)
        {
            printf("HTTPRequests::waitForRequest: request complete!\n");
            break;
        }
        cyw43_arch_poll();
        sleep_ms(5);

        // Check for timeout
        if (to_ms_since_boot(get_absolute_time()) - start_time > connectionTimeout_) 
        {
            printf("HTTPRequests::sendRequestToIp: Connection timed out\n");
            timeoutReached = true;
            result = ERR_TIMEOUT;
        }
    }

    if(!timeoutReached)
    {
        result = requestData.getServerResponse().err;
        printf("HTTPRequests::waitForRequest: result=%d\n", result);
        
    }

    printf("HTTPRequests::waitForRequest: end\n");
    return result;
}

void HTTPRequests::dnsFoundCallback(const char *name, const ip_addr_t *ipaddr, void *callback_arg)
{
    if (ipaddr)
    {
        printf("HTTPRequests::dnsFoundCallback: DNS resolved. IP: %s\n", ip4addr_ntoa(ipaddr));
        RequestData * requestData = (RequestData*)callback_arg;
        connect(ipaddr, requestData);
    } 
    else 
    {
        printf("HTTPRequests::dnsFoundCallback: DNS resolution failed\n");
    }
}

void HTTPRequests::connect(const ip_addr_t *ipaddr, RequestData * requestData)
{
    struct tcp_pcb *pcb = tcp_new();
    if (pcb != NULL) 
    {
        tcp_arg(pcb, requestData);
        tcp_recv(pcb, tcpClientReceived);
        tcp_err(pcb, tcpClientError);

        err_t connectErr = tcp_connect(pcb, ipaddr, requestData->getPort(), tcpClientConnected);
        if (connectErr != ERR_OK) 
        {
            printf("HTTPRequests::connect: Failed to connect: %d\n", connectErr);
            if(requestData != nullptr)
            {
                requestData->getServerResponse().err = connectErr;
                requestData->getServerResponse().requestComplete = true;
            }
            closeConnection(pcb);
        }
    } 
    else 
    {
        printf("HTTPRequests::connect: Failed to create TCP PCB\n");
    }
}

err_t HTTPRequests::tcpClientConnected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    RequestData * requestData = (RequestData*)arg;
    if (err != ERR_OK) 
    {
        if(requestData != nullptr)
        {
            requestData->getServerResponse().requestComplete = true;
        }
        printf("HTTPRequests::tcpClientConnected: Connect failed %d\n", err);
    }
    else
    {
        printf("HTTPRequests::tcpClientConnected: Connected\n");

        if(requestData != nullptr)
        {
            std::string request = requestData->getRequest();

            printf("HTTPRequests::tcpClientConnected: Writing request...\n");
            err = tcp_write(tpcb, request.c_str(), request.length(), TCP_WRITE_FLAG_COPY);
            if (err != ERR_OK) 
            {
                printf("HTTPRequests::tcpClientConnected: Failed to write data: %d\n", err);
            }
            else
            {
                err = tcp_output(tpcb);
                if (err != ERR_OK) 
                {
                    printf("HTTPRequests::tcpClientConnected: Failed to send data: %d\n", err);
                }
            }

            requestData->getServerResponse().err = err;
        }
        else
        {
            printf("HTTPRequests::tcpClientConnected: RequestData is nullptr \n");
            err = ERR_ARG;
        }
    }
   
    return err;
}


err_t HTTPRequests::tcpClientReceived(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    RequestData * requestData = (RequestData*)arg;
    if (err == ERR_OK && p != NULL) 
    {
        // Received response data
        printf("HTTPRequests::tcpClientReceived: Received response: %.*s\n", p->len, (char*)p->payload);
        tcp_recved(tpcb, p->tot_len);
        pbuf_free(p);
        if(requestData != nullptr)
        {
            ServerResponse & serverResponse = requestData->getServerResponse();
            serverResponse.payload = std::string(static_cast<char*>(p->payload), p->len);
        }
    } 
    else if (err == ERR_OK && p == NULL) 
    {
        printf("HTTPRequests::tcpClientReceived: Connection closed by remote host\n");
    }
    else
    {
        printf("HTTPRequests::tcpClientReceived: Error in tcpClientReceived: %d\n", err);
    }

    if(requestData != nullptr)
    {
        requestData->getServerResponse().requestComplete = true;
        requestData->getServerResponse().err = err;
        printf("HTTPRequests::tcpClientReceived: err=%d\n", err);
    }

    closeConnection(tpcb);
    return err;
}

void HTTPRequests::tcpClientError(void *arg, err_t err)
{
    printf("HTTPRequests::tcpClientError: TCP client error: %d\n", err);
    if (err == ERR_ABRT) 
    {
        printf("HTTPRequests::tcpClientError: Connection aborted\n");
    } 
    else if (err == ERR_RST) 
    {
        printf("HTTPRequests::tcpClientError: Connection reset\n");
    } else 
    {
        printf("HTTPRequests::tcpClientError: Other error\n");
    }

    RequestData * requestData = (RequestData*)arg;
    if(requestData != nullptr)
    {
        requestData->getServerResponse().requestComplete = true;
        requestData->getServerResponse().err = err;
        printf("HTTPRequests::tcpClientError: err=%d\n", err);
    }
}

void HTTPRequests::closeConnection(struct tcp_pcb *tpcb)
{
    if (tpcb != NULL) 
    {
        tcp_arg(tpcb, NULL);
        tcp_recv(tpcb, NULL);
        tcp_err(tpcb, NULL);
        tcp_abort(tpcb);
    }
    printf("HTTPRequests::closeConnection: Connection closed!\n");
}