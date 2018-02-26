//
// Created by Okada, Takahiro on 2018/02/04.
//

#include "Web3.h"
#include <WiFiClientSecure.h>
#include "CaCert.h"
#include "Log.h"
#include "Util.h"
#include "cJSON/cJSON.h"

WiFiClientSecure client;
Log debug;
#define LOG(x) debug.println(x)

char host[128];
char path[128];

Web3::Web3(const char* _host, const char* _path) {
    client.setCACert(infura_ca_cert);
    sprintf(host, "%s", _host);
    sprintf(path, "%s", _path);

}

void Web3::Web3ClientVersion(char* result) {
    char input[128];
    char output[128];
    memset(input,0,128);
    memset(output,0,128);
    generateJson(input, "web3_clientVersion", "[]");
    exec(input, output);
    getString(output, result);
}

// data length must be smaller than 256
void Web3::Web3Sha3(const char* data, char* result) {
    char input[256];
    char output[128];
    char param[258];
    memset(input,0,256);
    memset(output,0,128);
    memset(param,0,256);
    sprintf(param, "[\"%s\"]", data);
    generateJson(input, "web3_sha3", param);
    exec(input, output);
    getString(output, result);
}

int Web3::NetVersion() {
    int ret;
    char input[128];
    char result[128];
    memset(input,0,128);
    memset(result,0,128);
    generateJson(input, "net_version", "[]");
    exec(input, result);
    ret = getInt(result);
    return ret;
}

bool Web3::NetListening() {
    bool ret;
    char input[128];
    char result[128];
    memset(input,0,128);
    memset(result,0,128);
    generateJson(input, "net_listening", "[]");
    exec(input, result);
    ret = getBool(result);
    return ret;
}

int Web3::NetPeerCount() {
    int ret;
    char input[128];
    char result[128];
    memset(input,0,128);
    memset(result,0,128);
    generateJson(input, "net_peerCount", "[]");
    exec(input, result);
    ret = getInt(result);
    return ret;
}

double Web3::EthProtocolVersion() {
    double ret;
    char input[128];
    char result[128];
    memset(input,0,128);
    memset(result,0,128);
    generateJson(input, "eth_protocolVersion", "[]");
    exec(input, result);
    ret = getDouble(result);
    return ret;
}

bool Web3::EthSyncing() {
    bool ret;
    char input[128];
    char result[128];
    memset(input,0,128);
    memset(result,0,128);
    generateJson(input, "eth_syncing", "[]");
    exec(input, result);

    cJSON *root, *value;
    root = cJSON_Parse(result);
    value = cJSON_GetObjectItem(root, "result");
    if (cJSON_IsBool(value)) {
        ret = false;
    } else{
        ret = true;
    }
    cJSON_free(root);
    return ret;
}

bool Web3::EthMining() {
    bool ret;
    char input[128];
    char result[128];
    memset(input,0,128);
    memset(result,0,128);
    generateJson(input, "eth_mining", "[]");
    exec(input, result);

    ret = getBool(result);
    return ret;
}

double Web3::EthHashrate() {
    double ret;
    char input[128];
    char result[128];
    memset(input,0,128);
    memset(result,0,128);
    generateJson(input, "eth_hashrate", "[]");
    exec(input, result);

    ret = getDouble(result);
    return ret;
}

long long int Web3::EthGasPrice() {
    long long int ret;
    char input[128];
    char result[128];
    memset(input,0,128);
    memset(result,0,128);
    generateJson(input, "eth_gasPrice", "[]");
    exec(input, result);
    ret = getLongLong(result);
    return ret;
}

void Web3::EthAccounts(char** array, int size) {
     // TODO
}

int Web3::EthBlockNumber() {
    int ret;
    char input[128];
    char result[128];
    memset(input,0,128);
    memset(result,0,128);
    generateJson(input, "eth_blockNumber", "[]");
    exec(input, result);
    ret = getInt(result);
    return ret;
}

long long int Web3::EthGetBalance(char* address) {
    long long int ret;
    char input[128];
    char result[128];
    char param[258];
    memset(input,0,128);
    memset(result,0,128);
    memset(param,0,256);
    sprintf(param, "[\"%s\",\"latest\"]", address);
    generateJson(input, "eth_getBalance", param);
    exec(input, result);
    ret = getLongLong(result);
    return ret;
}

int Web3::EthGetTransactionCount(char* address) {
    int ret;
    char input[128];
    char result[128];
    char param[258];
    memset(input,0,128);
    memset(result,0,128);
    memset(param,0,256);
    sprintf(param, "[\"%s\",\"latest\"]", address);
    generateJson(input, "eth_getTransactionCount", param);
    exec(input, result);
    ret = getInt(result);
    return ret;
}

void Web3::EthCall(char* from, char* to, long gas, long gasPrice,
                   char* value, char* data, char* output) {
    // TODO use gas, gasprice and value
    int ret;
    char input[512]; // note: if sizes of input and param are 1024, it crashes.
    char param[512];
    memset(input,0,512);
    memset(param,0,512);
    sprintf(param, "[{\"from\":\"%s\",\"to\":\"%s\",\"data\":\"%s\"}, \"latest\"]", from, to, data);
    generateJson(input, "eth_call", param);
    exec(input, output);
}

void Web3::EthSendSignedTransaction(uint8_t* data, uint32_t dataLen, char* output) {
    // TODO use gas, gasprice and value
    int ret;
    Serial.printf("datalen: %d", dataLen);
    char input[dataLen*2+64]; // note: if sizes of input and param are 1024, it crashes.
    char tmp[dataLen*2+10];
    char param[dataLen*2+10];
    memset(input,0,256);
    memset(tmp,0,256);
    memset(param,0,256);
    Util::BufToString(tmp, data, dataLen);
    sprintf(param, "[\"%s\"]", tmp);
    generateJson(input, "eth_sendRawTransaction", param);

    LOG("-- sendRawTransaction --------");
    LOG(input);

    exec(input, output);
}

// -------------------------------
// Private

void Web3::generateJson(char* out, const char* method, const char* params) {
    sprintf(out, "{\"jsonrpc\":\"2.0\",\"method\":\"%s\",\"params\":%s,\"id\":0}", method, params);
}

void Web3::exec(const char* data, char* result) {
    // start connection
    LOG("\nStarting connection to server...");
    int connected = client.connect(host, 443);

    if (!connected)
        LOG("Connection failed!");
    else {
        LOG("Connected to server!");
        // Make a HTTP request:
        char strPost[128];
        char strHost[128];
        char strContentLen[32];
        sprintf(strPost, "POST %s HTTP/1.1", path);
        sprintf(strHost, "Host: %s", host);
        sprintf(strContentLen, "Content-Length: %d", strlen(data));
        client.println(strPost);
        client.println(strHost);
        client.println("Content-Type: application/json");
        client.println(strContentLen);
        client.println("Connection: close");
        client.println();
        client.println(data);

        while (client.connected()) {
            String line = client.readStringUntil('\n');
            LOG(line.c_str());
            if (line == "\r") {
                break;
            }
        }
        // if there are incoming bytes available
        // from the server, read them and print them:
        while (client.available()) {
            char c = client.read();
            sprintf(result, "%s%c", result, c);
        }
        LOG(result);

        client.stop();
    }
}

int Web3::getInt(const char* json) {
    int ret = -1;
    cJSON *root, *value;
    root = cJSON_Parse(json);
    value = cJSON_GetObjectItem(root, "result");
    char tmp[12];
    memset(tmp, 0, 12);
    if (cJSON_IsString(value)) {
        ret = strtol(value->valuestring, nullptr, 16);
    }
    cJSON_free(root);
    return ret;
}

long Web3::getLong(const char* json) {
    long ret = -1;
    cJSON *root, *value;
    root = cJSON_Parse(json);
    value = cJSON_GetObjectItem(root, "result");
    char tmp[12];
    memset(tmp, 0, 12);
    if (cJSON_IsString(value)) {
        ret = strtol(value->valuestring, nullptr, 16);
    }
    cJSON_free(root);
    return ret;
}

long long int Web3::getLongLong(const char* json) {
    long long int ret = -1;
    cJSON *root, *value;
    root = cJSON_Parse(json);
    value = cJSON_GetObjectItem(root, "result");
    char tmp[32];
    memset(tmp, 0, 32);
    if (cJSON_IsString(value)) {
        ret = strtoll(value->valuestring, nullptr, 16);
    }
    cJSON_free(root);
    return ret;
}

double Web3::getDouble(const char* json) {
    double ret = -1;
    cJSON *root, *value;
    root = cJSON_Parse(json);
    value = cJSON_GetObjectItem(root, "result");
    char tmp[8];
    memset(tmp, 0, 8);
    if (cJSON_IsString(value)) {
        LOG(value->valuestring);
        ret = strtof(value->valuestring, nullptr);
    }
    cJSON_free(root);
    return ret;
}

bool Web3::getBool(const char* json) {
    bool ret = false;
    cJSON *root, *value;
    root = cJSON_Parse(json);
    value = cJSON_GetObjectItem(root, "result");
    if (cJSON_IsBool(value)) {
        ret = (bool)value->valueint;
    }
    cJSON_free(root);
    return ret;
}

void Web3::getString(const char* json, char* output) {
    cJSON *root, *value;
    root = cJSON_Parse(json);
    value = cJSON_GetObjectItem(root, "result");
    if (cJSON_IsString(value)) {
        strcpy(output, value->valuestring);
    }
    cJSON_free(root);

}

