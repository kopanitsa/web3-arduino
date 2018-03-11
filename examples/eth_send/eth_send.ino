#include <WiFi.h>
#include <Web3.h>
#include <Contract.h>

#define USE_SERIAL Serial

#define ENV_SSID     "<YOUR_SSID>"
#define ENV_WIFI_KEY "<YOUR_PASSWORD>"
#define MY_ADDRESS "0x<MY_ADDRESS>"
#define CONTRACT_ADDRESS "0x<CONTRACT_ADDRESS>"
#define INFURA_HOST "rinkeby.infura.io"
#define INFURA_PATH "/<YOUR_INFURA_ID>"

const char PRIVATE_KEY[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };
Web3 web3(INFURA_HOST, INFURA_PATH);

void eth_send_example();

void setup() {
    USE_SERIAL.begin(115200);

    for(uint8_t t = 4; t > 0; t--) {
        USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
        USE_SERIAL.flush();
        delay(1000);
    }

    WiFi.begin(ENV_SSID, ENV_WIFI_KEY);

    // attempt to connect to Wifi network:
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        // wait 1 second for re-trying
        delay(1000);
    }

    USE_SERIAL.println("Connected");

    eth_send_example();
}

void loop() {
    // put your main code here, to run repeatedly:
}

void eth_send_example() {
    Contract contract(&web3, CONTRACT_ADDRESS);
    contract.SetPrivateKey((uint8_t*)PRIVATE_KEY);
    uint32_t nonceVal = (uint32_t)web3.EthGetTransactionCount((char *)MY_ADDRESS);

    uint32_t gasPriceVal = 141006540;
    uint32_t  gasLimitVal = 3000000;
    uint8_t toStr[] = CONTRACT_ADDRESS;
    uint8_t valueStr[] = "0x00";
    uint8_t dataStr[100];
    memset(dataStr, 0, 100);
    string func = "set(uint256)";
    string p = contract.SetupContractData(&func, 123);
    string result = contract.SendTransaction(nonceVal, gasPriceVal, gasLimitVal, &toStr, &valueStr, &p);

    USE_SERIAL.println(result);
}
