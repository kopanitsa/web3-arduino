//
// Created by Okada, Takahiro on 2018/02/04.
//

#ifndef ARDUINO_WEB3_WEB3_H
#define ARDUINO_WEB3_WEB3_H

#include "stdint.h"
#include <string>

using namespace std;

class Web3 {
public:
    Web3(string* _host, string* _path);
    string Web3ClientVersion();
    string Web3Sha3(const string* data);
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
    long long int EthGetBalance(string* address);
    int EthGetTransactionCount(string* address);

    string EthCall(string* from, string* to, long gas, long gasPrice, string* value, string* data);
    string EthSendSignedTransaction(string* data, uint32_t dataLen);

private:
    string exec(string* data);
    string generateJson(string* method, string* params);
    int getInt(const string* json);
    long getLong(const string* json);
    long long int getLongLong(const string* json);
    double getDouble(const string* json);
    bool getBool(const string* json);
    string getString(const string* json);

private:
    string* host;
    string* path;


};

#endif //ARDUINO_WEB3_WEB3_H
