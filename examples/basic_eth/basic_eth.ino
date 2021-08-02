#include <Arduino.h>
#include <WiFi.h>
#include <Web3.h>
#include <esp_system.h>
#include <esp_log.h>


#define USE_SERIAL Serial


#define ENV_SSID     "<YOUR_SSID>"
#define ENV_WIFI_KEY "<YOUR_PASSWORD>"
#define INFURA_HOST "rinkeby.infura.io"
#define INFURA_PATH "/<YOUR_INFURA_ID>"

Web3 web3(new std::string(INFURA_HOST), new std::string( INFURA_PATH));

void net_example();

String getMacAddress() {
  uint8_t baseMac[6];
  // Get MAC address for WiFi station
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  char baseMacChr[18] = {0};
  sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  return String(baseMacChr);
}

void setup() {
    USE_SERIAL.begin(115200);

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    WiFi.begin(ENV_SSID, ENV_WIFI_KEY);

    String mac = getMacAddress();
    USE_SERIAL.println(mac);

    // attempt to connect to Wifi network:
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        // wait 1 second for re-trying
        delay(1000);
    }

    USE_SERIAL.println("Connected");

    net_example();
}

void loop() {
  // put your main code here, to run repeatedly:
}

void net_example() {
    int version = web3.NetVersion();
    USE_SERIAL.println("net_version");
    USE_SERIAL.println(version); // this is 4

    bool listening = web3.NetListening();
    USE_SERIAL.println("net_listening");
    if (listening) {
        USE_SERIAL.println("listening");
    } else{
        USE_SERIAL.println("not listening");
    }

    int peerCount = web3.NetPeerCount();
    USE_SERIAL.println("net_peerCount");
    USE_SERIAL.println(peerCount);
}
