//
// Created by Okada, Takahiro on 2018/02/04.
//

#ifndef SMARTOUTLET_WEB3_H
#define SMARTOUTLET_WEB3_H

#include "stdint.h"

class Web3 {
public:
    Web3(const char* host, const char* path);
    void Web3ClientVersion(char* result);
    void Web3Sha3(const char* data, char* result);
    int NetVersion();
    bool NetListening();
    int NetPeerCount();
    double EthProtocolVersion();
    bool EthSyncing();
    bool EthMining();
    double EthHashrate();
    long long int EthGasPrice();
    void EthAccounts(char** array, int size);
    int EthBlockNumber();
    long long int EthGetBalance(char* address);
    int EthGetTransactionCount(char* address);

    void EthCall(char* from, char* to, long gas, long gasPrice, char* value, char* data, char* output);
    void EthSendSignedTransaction(uint8_t* data, uint32_t dataLen, char* output);

private:
    void exec(const char* data, char* result);
    void generateJson(char* out, const char* method, const char* params);
    int getInt(const char* json);
    long getLong(const char* json);
    long long int getLongLong(const char* json);
    double getDouble(const char* json);
    bool getBool(const char* json);
    void getString(const char* json, char* output);

};

#endif //SMARTOUTLET_WEB3_H
