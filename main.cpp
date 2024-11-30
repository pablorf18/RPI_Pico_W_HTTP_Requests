////////////////////////////////////////////////////////////
// Example of GET and POST request to an url and to an ip //
////////////////////////////////////////////////////////////

#include "HTTPRequests.h"

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define SERVER_URL ""
#define SERVER_IP ""
#define SERVER_PORT 80

bool connectToWifi()
{
    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 30000)) 
    {
        printf("Failed to connect to Wi-Fi\n");
        return false;
    }
    printf("Connected to Wi-Fi\n");
    return true;
}

int main() 
{
    stdio_init_all();
    sleep_ms(5000);  // Wait for serial console to connect
    printf("Pico W HTTP Client Example\n");

    if (cyw43_arch_init()) 
    {
        printf("Failed to initialize cyw43_arch\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    int retries = 3;
    bool connected = false;
    //sometimes the connection to wifi fails, so this scope will try it again
    while(!connected && retries > 0)
    {
        connected = connectToWifi();
        retries--;
    }

    if(!connected)
    {
        printf("Failed connecting to wifi\n");
        return 1;
    }

    //convert the ip (string) to an ip_addr_t
    ip_addr_t ip;
    ipaddr_aton(SERVER_IP, &ip);

    HTTPRequests httpRequests;
    std::string simpleMessage = "TestMessage";

    //test the GET request. Take care, there is a /data address in the example's server, but
    //you can change to / if your server does not.
    std::string request = "GET /data?message=" + simpleMessage + " HTTP/1.1\r\n"
                          "Host: " + std::string(SERVER_URL) + "\r\n"
                          "Connection: close\r\n"
                          "\r\n";
    
    auto getResult = httpRequests.sendRequestToUrl(SERVER_URL, SERVER_PORT, request);
    printf("GET request to url result %d\n", getResult.first);
    printf("GET to url, Server response %s\n", getResult.second.payload.c_str());

    sleep_ms(1000);

    request = "GET /data?message=" + simpleMessage + " HTTP/1.1\r\n"
              "Host: " + std::string(SERVER_IP) + "\r\n"
              "Connection: close\r\n"
              "\r\n";

    auto getIpResult = httpRequests.sendRequestToIp(ip, SERVER_PORT, request);
    printf("GET request to ip result %d\n", getIpResult.first);
    printf("GET to ip, Server response %s\n", getIpResult.second.payload.c_str());

    sleep_ms(1000);

    //test the POST request. Take care, there is a /data address in the example's server, but
    //you can change to / if your server does not.
    request = "POST /data HTTP/1.1\r\n"
              "Host: " + std::string(SERVER_URL) + "\r\n"
              "Content-Type: application/x-www-form-urlencoded\r\n"
              "Content-Length: " + std::to_string(simpleMessage.length()) + "\r\n"
              "\r\n" + simpleMessage;
    
    auto postResult = httpRequests.sendRequestToUrl(SERVER_URL, SERVER_PORT, request);
    printf("POST request to url result %d\n", postResult.first);
    printf("POST to url, Server response %s\n", postResult.second.payload.c_str());

    sleep_ms(1000);

    request = "POST /data HTTP/1.1\r\n"
              "Host: " + std::string(SERVER_IP) + "\r\n"
              "Content-Type: application/x-www-form-urlencoded\r\n"
              "Content-Length: " + std::to_string(simpleMessage.length()) + "\r\n"
              "\r\n" + simpleMessage;

    auto postIpResult = httpRequests.sendRequestToIp(ip, SERVER_PORT, request);
    printf("POST request to ip result %d\n", postIpResult.first);
    printf("POST to ip, Server response %s\n", postIpResult.second.payload.c_str());

    cyw43_arch_deinit();

    printf("End of program\n");
    return 0;
}