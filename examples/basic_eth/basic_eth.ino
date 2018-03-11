#include <WiFi.h>
#include <Web3.h>

#define USE_SERIAL Serial

#define ENV_SSID     "<YOUR_SSID>"
#define ENV_WIFI_KEY "<YOUR_PASSWORD>"
#define INFURA_HOST "rinkeby.infura.io"
#define INFURA_PATH "/<YOUR_INFURA_ID>"
#define ADDRESS "0x<YOUR_ETH_ADDRESS>"

Web3 web3(INFURA_HOST, INFURA_PATH);

void eth_example();

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

    eth_example();
}

void loop() {
    // put your main code here, to run repeatedly:
}

void eth_example() {
    char tmp[32];

    double version = web3.EthProtocolVersion();
    USE_SERIAL.println("eth_protocolVersion");
    USE_SERIAL.println(version);

    bool listening = web3.EthSyncing();
    USE_SERIAL.println("eth_syncing");
    if (listening) {
        USE_SERIAL.println("syncing");
    } else{
        USE_SERIAL.println("not syncing");
    }

    bool mining = web3.EthMining();
    USE_SERIAL.println("eth_mining");
    if (mining) {
        USE_SERIAL.println("mining");
    } else{
        USE_SERIAL.println("not mining");
    }

    double hashrate = web3.EthHashrate();
    USE_SERIAL.println("eth_hashrate");
    USE_SERIAL.println(hashrate);

    long long int gasPrice = web3.EthGasPrice();
    USE_SERIAL.println("eth_gasPrice");
    memset(tmp, 0, 32);
    sprintf(tmp, "%lld", gasPrice);
    USE_SERIAL.println(tmp);

    int blockNumber = web3.EthBlockNumber();
    USE_SERIAL.println("eth_blockNumber");
    memset(tmp, 0, 32);
    sprintf(tmp, "%d", blockNumber);
    USE_SERIAL.println(tmp);

    string address = "0xd7049ea6f47ef848C0Ad570dbA618A9f6e4Eb25C";
    long long int balance = web3.EthGetBalance(&address);
    USE_SERIAL.println("eth_getBalance");
    memset(tmp, 0, 32);
    sprintf(tmp, "%lld", balance);
    USE_SERIAL.println(tmp);

    int txcount = web3.EthGetTransactionCount(&address);
    USE_SERIAL.println("eth_getTransactionCount");
    memset(tmp, 0, 32);
    sprintf(tmp, "%d", txcount);
    USE_SERIAL.println(tmp);
}
