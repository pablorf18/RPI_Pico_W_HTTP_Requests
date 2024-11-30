Library to simplify HTTP request to a server using a RPI Pico W.

The main.cpp is an example of use. You only have to fill the initial defines.
#define WIFI_SSID "" -> your wifi SSID
#define WIFI_PASSWORD "" -> your wifi password
#define SERVER_URL "" -> the server url, without the initial http://. E.g: github.com
#define SERVER_IP "" -> the server ip if you are working with your own server and you don't have a domain. E.g: 192.168.1.10
#define SERVER_PORT 80

To build the project:
1. Export the pico sdk env variable: export PICO_SDK_PATH=../../pico-sdk -> take care and point to your pico-sdk right folder
2. mkdir build && cd build
3. cmake -DPICO_BOARD=pico_w ..
4. make