#include <WiFi.h>
#include <Web3.h>

#define USE_SERIAL Serial

#define ENV_SSID     "<YOUR_SSID>"
#define ENV_WIFI_KEY "<YOUR_PASSWORD>"
#define INFURA_HOST "rinkeby.infura.io"
#define INFURA_PATH "/<YOUR_INFURA_ID>"

Web3 web3(INFURA_HOST, INFURA_PATH);

void web3_example();

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

    web3_example();
}

void loop() {
    // put your main code here, to run repeatedly:
}

void web3_example() {
    string result = web3.Web3ClientVersion();
    USE_SERIAL.println("web3_ClientVersion");
    USE_SERIAL.println(result.c_str());

    string src = "0x68656c6c6f20776f726c64";
    result = web3.Web3Sha3(&src);
    USE_SERIAL.println("web3_sha3");
    USE_SERIAL.println(result.c_str()); // 0x47173285a8d7341e5e972fc677286384f802f8ef42a5ec5f03bbfa254cb01fad
}
