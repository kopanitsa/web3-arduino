#include <Arduino.h>
#include <WiFi.h>
#include <Web3.h>
#include <Contract.h>
#include <esp_system.h>
#include <esp_log.h>

#define USE_SERIAL Serial

#define ENV_SSID     "<YOUR_SSID>"
#define ENV_WIFI_KEY "<YOUR_PASSWORD>"
#define MY_ADDRESS "0x<MY_ADDRESS>"
#define CONTRACT_ADDRESS "0x<CONTRACT_ADDRESS>"
#define INFURA_HOST "rinkeby.infura.io"
#define INFURA_PATH "/<YOUR_INFURA_ID>"

Web3 web3(new std::string(INFURA_HOST), new std::string( INFURA_PATH));

void eth_call_example();

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

    eth_call_example();
}

void loop() {
    // put your main code here, to run repeatedly:
}

void eth_call_example() {
    Contract contract(&web3, new std::string(CONTRACT_ADDRESS));
    strcpy(contract.options.from, MY_ADDRESS);
    strcpy(contract.options.gasPrice,"2000000000000000");
    contract.options.gas = 5000000;
    string func = "getBalance()";
    string param = contract.SetupContractData(&func);
    string result = contract.Call(&param);
    USE_SERIAL.println("result");
    USE_SERIAL.println(result.c_str());
}
