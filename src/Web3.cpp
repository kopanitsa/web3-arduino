//
// Created by Okada, Takahiro on 2018/02/04.
//

#include "Web3.h"
#include <WiFiClientSecure.h>
#include "CaCert.h"
#include "Log.h"
#include "Util.h"
#include "cJSON/cJSON.h"
#include <iostream>
#include <sstream>

WiFiClientSecure client;
Log debug;
#define LOG(x) debug.println(x)

Web3::Web3(string* _host, string* _path) {
    client.setCACert(infura_ca_cert);
    host = _host;
    path = _path;

}

string Web3::Web3ClientVersion() {
    string m = "web3_clientVersion";
    string p = "[]";
    string input = generateJson(&m, &p);
    string output = exec(&input);
    return getString(&output);
}

string Web3::Web3Sha3(const string* data) {
    string m = "web3_sha3";
    string p = "[\"" + *data + "\"]";
    string input = generateJson(&m, &p);
    string output = exec(&input);
    return getString(&output);
}

int Web3::NetVersion() {
    string m = "net_version";
    string p = "[]";
    string input = generateJson(&m, &p);
    string output = exec(&input);
    return getInt(&output);
}

bool Web3::NetListening() {
    string m = "net_listening";
    string p = "[]";
    string input = generateJson(&m, &p);
    string output = exec(&input);
    return getBool(&output);
}

int Web3::NetPeerCount() {
    string m = "net_peerCount";
    string p = "[]";
    string input = generateJson(&m, &p);
    string output = exec(&input);
    return getInt(&output);
}

double Web3::EthProtocolVersion() {
    string m = "eth_protocolVersion";
    string p = "[]";
    string input = generateJson(&m, &p);
    string output = exec(&input);
    return getDouble(&output);
}

bool Web3::EthSyncing() {
    string m = "eth_syncing";
    string p = "[]";
    string input = generateJson(&m, &p);
    string result = exec(&input);

    cJSON *root, *value;
    root = cJSON_Parse(result.c_str());
    value = cJSON_GetObjectItem(root, "result");
    bool ret;
    if (cJSON_IsBool(value)) {
        ret = false;
    } else{
        ret = true;
    }
    cJSON_free(root);
    return ret;
}

bool Web3::EthMining() {
    string m = "eth_mining";
    string p = "[]";
    string input = generateJson(&m, &p);
    string output = exec(&input);
    return getBool(&output);
}

double Web3::EthHashrate() {
    string m = "eth_hashrate";
    string p = "[]";
    string input = generateJson(&m, &p);
    string output = exec(&input);
    return getDouble(&output);
}

long long int Web3::EthGasPrice() {
    string m = "eth_gasPrice";
    string p = "[]";
    string input = generateJson(&m, &p);
    string output = exec(&input);
    return getLongLong(&output);
}

void Web3::EthAccounts(char** array, int size) {
     // TODO
}

int Web3::EthBlockNumber() {
    string m = "eth_blockNumber";
    string p = "[]";
    string input = generateJson(&m, &p);
    string output = exec(&input);
    return getInt(&output);
}

long long int Web3::EthGetBalance(string* address) {
    string m = "eth_getBalance";
    string p = "[\"" + *address + "\",\"latest\"]";
    string input = generateJson(&m, &p);
    string output = exec(&input);
    return getLongLong(&output);
}

int Web3::EthGetTransactionCount(string* address) {
    string m = "eth_getTransactionCount";
    string p = "[\"" + *address + "\",\"latest\"]";
    string input = generateJson(&m, &p);
    string output = exec(&input);
    return getInt(&output);
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
//    generateJson(input, "eth_call", param);
//    exec(input, output);
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
//    generateJson(input, "eth_sendRawTransaction", param);

#if 0
    LOG(input);
#endif
//    exec(input, output);
}

// -------------------------------
// Private

string Web3::generateJson(string* method, string* params) {
    return "{\"jsonrpc\":\"2.0\",\"method\":\"" + *method + "\",\"params\":" + *params + ",\"id\":0}";
}

string Web3::exec(string* data) {
    string result;

    // start connection
    LOG("\nStarting connection to server...");
    int connected = client.connect(host->c_str(), 443);
    if (!connected) {
        return "";
    }

    LOG("Connected to server!");
    // Make a HTTP request:
    int l = data->size();
    stringstream ss;
    ss << l;
    string lstr = ss.str();

    string strPost = "POST " + *path + " HTTP/1.1";
    string strHost = "Host: " + *host;
    string strContentLen = "Content-Length: " + lstr;

    client.println(strPost.c_str());
    client.println(strHost.c_str());
    client.println("Content-Type: application/json");
    client.println(strContentLen.c_str());
    client.println("Connection: close");
    client.println();
    client.println(data->c_str());

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
        result += c;
    }
    LOG(result.c_str());

    client.stop();

    return result;
}

int Web3::getInt(const string* json) {
    int ret = -1;
    cJSON *root, *value;
    root = cJSON_Parse(json->c_str());
    value = cJSON_GetObjectItem(root, "result");
    if (cJSON_IsString(value)) {
        ret = strtol(value->valuestring, nullptr, 16);
    }
    cJSON_free(root);
    return ret;
}

long Web3::getLong(const string* json) {
    long ret = -1;
    cJSON *root, *value;
    root = cJSON_Parse(json->c_str());
    value = cJSON_GetObjectItem(root, "result");
    if (cJSON_IsString(value)) {
        ret = strtol(value->valuestring, nullptr, 16);
    }
    cJSON_free(root);
    return ret;
}

long long int Web3::getLongLong(const string* json) {
    long long int ret = -1;
    cJSON *root, *value;
    root = cJSON_Parse(json->c_str());
    value = cJSON_GetObjectItem(root, "result");
    if (cJSON_IsString(value)) {
        ret = strtoll(value->valuestring, nullptr, 16);
    }
    cJSON_free(root);
    return ret;
}

double Web3::getDouble(const string* json) {
    double ret = -1;
    cJSON *root, *value;
    root = cJSON_Parse(json->c_str());
    value = cJSON_GetObjectItem(root, "result");
    if (cJSON_IsString(value)) {
        LOG(value->valuestring);
        ret = strtof(value->valuestring, nullptr);
    }
    cJSON_free(root);
    return ret;
}

bool Web3::getBool(const string* json) {
    bool ret = false;
    cJSON *root, *value;
    root = cJSON_Parse(json->c_str());
    value = cJSON_GetObjectItem(root, "result");
    if (cJSON_IsBool(value)) {
        ret = (bool)value->valueint;
    }
    cJSON_free(root);
    return ret;
}

string Web3::getString(const string* json) {
    cJSON *root, *value;
    root = cJSON_Parse(json->c_str());
    value = cJSON_GetObjectItem(root, "result");
    if (cJSON_IsString(value)) {
        cJSON_free(root);
        return string(value->valuestring);
    }
    cJSON_free(root);
    return nullptr;
}
