//
// Created by Okada, Takahiro on 2018/02/05.
//

#ifndef ARDUINO_WEB3_CONTRACT_H
#define ARDUINO_WEB3_CONTRACT_H

#include "Arduino.h"
#include "Log.h"
#include "Web3.h"

class Contract {

public:
    typedef struct {
        char from[80];
        char to[80];
        char gasPrice[20];
        long gas;
    } Options;
    Options options;

public:
    Contract(Web3* _web3, const string* address);
    void SetPrivateKey(const uint8_t *key);
    string SetupContractData(const string *func, ...);
    string Call(const string* param);
    string SendTransaction(uint32_t nonceVal, uint32_t gasPriceVal, uint32_t gasLimitVal,
                           string *toStr, string *valueStr, string *dataStr);

private:
    Log Debug;
    #define LOG(x) Debug.println(x)

    Web3* web3;
    const string * contractAddress;
    const uint8_t * privateKey;

private:
    string GenerateContractBytes(const string *func);
    string GenerateBytesForInt(const int32_t value);
    string GenerateBytesForUint(const uint32_t value);
    string GenerateBytesForAddress(const string *value);
    string GenerateBytesForString(const string *value);
    string GenerateBytesForBytes(const char* value, const int len);

    void SetupTransactionImpl1(uint8_t* signature, int* recid, uint32_t nonceVal, uint32_t gasPriceVal, uint32_t  gasLimitVal,
                                         uint8_t* toStr, uint8_t* valueStr, uint8_t* dataStr);
    uint32_t SetupTransactionImpl2(uint8_t* out,
                                             uint32_t nonceVal, uint32_t gasPriceVal, uint32_t  gasLimitVal,
                                             uint8_t* toStr, uint8_t* valueStr, uint8_t* dataStr, uint8_t* signature, uint8_t recid);
    uint32_t RlpEncode(uint8_t* encoded,
                       uint32_t nonceVal, uint32_t gasPriceVal, uint32_t  gasLimitVal,
                       uint8_t* toStr, uint8_t* valueStr, uint8_t* dataStr);
    uint32_t RlpEncodeForRawTransaction(uint8_t* encoded,
                                        uint32_t nonceVal, uint32_t gasPriceVal, uint32_t  gasLimitVal,
                                        uint8_t* toStr, uint8_t* valueStr, uint8_t* dataStr, uint8_t* sig, uint8_t recid);
    void Sign(uint8_t* hash, uint8_t* sig, int* recid);
};


#endif //ARDUINO_WEB3_CONTRACT_H
