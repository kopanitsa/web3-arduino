//
// Created by Okada, Takahiro on 2018/02/03.
//
// Reference:
//     Arduino sample code (WiFiClientSecure)
//

#include <Arduino.h>
#include <WiFi.h>
#include <web3/Contract.h>
#include "Conf.h"
#include "web3/Web3.h"
#include "web3/Contract.h"

#define USE_SERIAL Serial

Web3 web3(INFURA_HOST, INFURA_PATH);

void testWeb3();
void testNet();
void testEth1();
void testEth2();
void testEth3();

void setup() {

    USE_SERIAL.begin(115200);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

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
    testWeb3();
    testNet();
    testEth1();
    testEth2();
    testEth3();
}

void testWeb3() {
    char result[128];

    memset(result, 0, 128);
    web3.Web3ClientVersion(result);
    USE_SERIAL.println("web3_ClientVersion");
    USE_SERIAL.println(result);

    memset(result, 0, 128);
    web3.Web3Sha3("0x68656c6c6f20776f726c64", result);
    USE_SERIAL.println("web3_sha3");
    USE_SERIAL.println(result); // 0x47173285a8d7341e5e972fc677286384f802f8ef42a5ec5f03bbfa254cb01fad
}

void testNet() {
    char result[128];

    int version = web3.NetVersion();
    USE_SERIAL.println("net_version");
    USE_SERIAL.println(version);

    bool listening = web3.NetListening();
    USE_SERIAL.println("net_listening");
    if (listening) {
        USE_SERIAL.println("listening");
    } else{
        USE_SERIAL.println("not listening");
    }

    memset(result, 0, 128);
    int peerCount = web3.NetPeerCount();
    USE_SERIAL.println("net_peerCount");
    USE_SERIAL.println(peerCount);
}

void testEth1() {
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

    long long int balance = web3.EthGetBalance((char*)"0xd7049ea6f47ef848C0Ad570dbA618A9f6e4Eb25C");
    USE_SERIAL.println("eth_getBalance");
    memset(tmp, 0, 32);
    sprintf(tmp, "%lld", balance);
    USE_SERIAL.println(tmp);

    int txcount = web3.EthGetTransactionCount((char*)"0xd7049ea6f47ef848C0Ad570dbA618A9f6e4Eb25C");
    USE_SERIAL.println("eth_getBalance");
    memset(tmp, 0, 32);
    sprintf(tmp, "%d", txcount);
    USE_SERIAL.println(tmp);

}

void testEth2() {
    char result[512];
    memset(result, 0, 512);
    web3.EthCall((char*)"0xd7049ea6f47ef848c0ad570dba618a9f6e4eb25c"
            ,(char*)"0x018968e41da1364e328499613a7e5a22904ad513"
            ,0, 0, (char*)""
            ,(char*)"0x30cc3597d7049ea6f47ef848c0ad570dba618a9f6e4eb25c"
            ,result);
    USE_SERIAL.println("eth_call");
    USE_SERIAL.println(result);

    memset(result, 0, 512);
    Contract contract(&web3, "0x018968e41da1364e328499613a7e5a22904ad513");
    strcpy(contract.options.from,"0xd7049ea6f47ef848c0ad570dba618a9f6e4eb25c");
    strcpy(contract.options.gasPrice,"20000000000000");
    contract.options.gas = 5000000;
    contract.SetupContractData(result, "buyCoin()");
    contract.Call(result);
    USE_SERIAL.println(result);
    memset(result, 0, 512);
    contract.SetupContractData(result, "hogeuint(uint8,uint16,uint32,uint256,uint)", 1, 12345, 1003, 4, 5);
    contract.Call(result);
    USE_SERIAL.println(result);
    memset(result, 0, 512);
    contract.SetupContractData(result, "hogeint(int8,int16,int32,int256,int)", -1, 12345, -1003, 4, -5);
    contract.Call(result);
    USE_SERIAL.println(result);
    memset(result, 0, 512);
    contract.SetupContractData(result, "hogeaddress(address)", "0xd7049ea6f47ef848C0Ad570dbA618A9f6e4Eb25C");
    contract.Call(result);
    USE_SERIAL.println(result);
    memset(result, 0, 512);
    contract.SetupContractData(result, "hogebool(bool,bool)", true, false);
    contract.Call(result);
    USE_SERIAL.println(result);
    memset(result, 0, 512);
    contract.SetupContractData(result, "hogestring(string)", "Hello, world!");
    contract.Call(result);
    USE_SERIAL.println(result);
    memset(result, 0, 512);
    contract.SetupContractData(result, "hogebytes(bytes5,bytes12)", "hello", "123456789012");
    contract.Call(result);
    USE_SERIAL.println(result);

}

void testEth3() {
    // send raw transaction
    char result[128];
    memset(result, 0, 128);
    uint8_t data[] = {0x31,0x32,0x33};
    web3.EthSendSignedTransaction(data, 3, result);
    USE_SERIAL.println("EthSendSignedTransaction");
    USE_SERIAL.println(result);

    // sign transaction
    Contract contract(&web3, "0x018968e41da1364e328499613a7e5a22904ad513");
    contract.setPrivateKey(<your_private_key>);
    uint32_t nonceVal = (uint32_t)web3.EthGetTransactionCount((char *)"0xd7049ea6f47ef848c0ad570dba618a9f6e4eb25c");
    uint32_t gasPriceVal = 2000000000;
    uint32_t  gasLimitVal = 3000000;
    uint8_t toStr[] = "e759aab0343e7d4c9e23ac5760a12ed9d9af4421";
    uint8_t valueStr[] = "0x10";
    uint8_t dataStr[] = "0x60fe47b10000000000000000000000000000000000000000000000000000000000000064";
    contract.SetupTransaction(nonceVal, gasLimitVal, gasLimitVal, toStr, valueStr, dataStr);

}

void loop() {
    delay(5000);
}


