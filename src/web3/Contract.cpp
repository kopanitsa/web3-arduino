//
// Created by Okada, Takahiro on 2018/02/05.
//

#include "Contract.h"
#include "Web3.h"
#include <WiFi.h>
#include "Util.h"
#include "Log.h"
#include "micro-ecc/uECC.h"
#include "bearssl/inc/bearssl_hash.h"

Contract::Contract(Web3* _web3, const char* address) {
    web3 = _web3;
    contractAddress = address;
    options.gas=0;
    strcpy(options.from,"");
    strcpy(options.to,"");
    strcpy(options.gasPrice,"0");
}

void Contract::setPrivateKey(uint8_t* key) {
    privateKey = key;
}

// --------------------------------------------
// Development Status
// supported types:
//    uint<M>, (0<M<256, M%8=0)
//    uint,  (=uint256)
//    int<M>, (0<M<256, M%8=0)
//    int,  (=int256)
//    address,  (=uint160)
//    bool,
//    bytes<M> (0<M<=32)
//    function, (=bytes24)
//    string,
// not supported yet:
//    fixed<M>x<N>,
//    ufixed<M>x<N>,
//    <type>[M],
//    bytes,
//    <type>[],
// --------------------------------------------
void Contract::SetupContractData(char *out, const char *func, ...) {
    // addressの生成
    char contractBytes[11];
    memset(contractBytes, 0, 11);
    GenerateContractBytes(func, contractBytes);
    LOG("==============");
    LOG(contractBytes);

    // funcの文字列をdecodeして、どの型の引数がいくつあるかを解析
    size_t paramCount = 0;
    char params[8][20]; // 8個の20文字以内の引数を格納可能
    memset(params, 0, 20*8);
    char *p;
    char tmp[64];
    memset(tmp, 0, 64);
    strcpy(tmp, func);
    strtok(tmp, "(");
    p = strtok(0, "(");
    p = strtok(p, ")");
    p = strtok(p, ",");
    if (p != 0 && strlen(p) < 20) {
        strcpy(params[paramCount++], p);
    }
    while(p != 0) {
        p = strtok(0, ",");
        if (p != 0 && strlen(p) < 20) {
            strcpy(params[paramCount++], p);
        }
    }
    LOG("==============");

    // 上の個数と型に応じてbyte列を生成
    va_list args;
    va_start(args, paramCount);
    for( int i = 0; i < paramCount; ++i ) {
        LOG(params[i]);
        if (strncmp(params[i], "uint", 4) == 0) {
            char output[64+2+1];
            memset(output, 0, 64+2+1);
            GenerateBytesForUint(output, va_arg(args, uint32_t));
            sprintf(out, "%s%s", out, output);
        } else if (strncmp(params[i], "int", 3) == 0
                   || strncmp(params[i], "bool", 4) == 0) {
            char output[64+2+1];
            memset(output, 0, 64+2+1);
            GenerateBytesForInt(output, va_arg(args, int32_t));
            sprintf(out, "%s%s", out, output);
        } else if (strncmp(params[i], "address", 7) == 0) {
            char output[64+2+1];
            memset(output, 0, 64+2+1);
            GenerateBytesForAddress(output, va_arg(args, char*));
            sprintf(out, "%s%s", out, output);
        } else if (strncmp(params[i], "string", 6) == 0) {
            char output[64+2+1];
            memset(output, 0, 64+2+1);
            GenerateBytesForString(output, va_arg(args, char*));
            sprintf(out, "%s%s", out, output);
        } else if (strncmp(params[i], "bytes", 5) == 0) {
            char output[64+2+1];
            memset(output, 0, 64+2+1);
            long len = strtol(params[i]+5, nullptr,10);
            GenerateBytesForBytes(output, va_arg(args, char*), len);
            sprintf(out, "%s%s", out, output);
        }
    }
    va_end(args);
}

void Contract::Call(char* param) {
    char result[512];
    memset(result, 0, 512);
    long gasPrice = strtol(options.gasPrice, nullptr, 10);
    web3->EthCall(options.from
            ,(char*)contractAddress
            ,options.gas, gasPrice, (char*)""
            ,param
            ,result);
    LOG(result);
}

void Contract::SetupTransaction(uint32_t nonceVal, uint32_t gasPriceVal, uint32_t  gasLimitVal,
                                uint8_t* toStr, uint8_t* valueStr, uint8_t* dataStr) {

    uint8_t signature[64];
    SetupTransactionImpl1(signature, nonceVal, gasPriceVal, gasLimitVal, toStr, valueStr, dataStr);
    printf("\nsig\n");
    for (int j=0; j<64; j++) {
        printf("%02x ", signature[j]);
    }
    printf("\n");
    SetupTransactionImpl2(nonceVal, gasPriceVal, gasLimitVal, toStr, valueStr, dataStr, signature);


    // send raw transaction
}

void Contract::SetupTransactionImpl1(uint8_t* signature, uint32_t nonceVal, uint32_t gasPriceVal, uint32_t  gasLimitVal,
                                uint8_t* toStr, uint8_t* valueStr, uint8_t* dataStr) {
    // encode
    uint8_t encoded[256];
    memset(encoded,0,256);
    char tmp[256];
    memset(tmp,0,256);
    uint32_t encoded_len = RlpEncode(encoded, nonceVal, gasLimitVal, gasLimitVal, toStr, valueStr, dataStr);

    // hash
    char hashedStr[256];
    memset(hashedStr,0,256);
    Util::BufToString(tmp, encoded, encoded_len);
    web3->Web3Sha3((char*)tmp, (char*)hashedStr);
    printf("hash: %s\n", hashedStr);

    // sign
    memset(tmp,0,256);
    Util::BufToString(tmp, (uint8_t *)hashedStr, 32);
    memset(signature,0,64);
    Sign((uint8_t*)tmp, signature);
}

void Contract::SetupTransactionImpl2(uint32_t nonceVal, uint32_t gasPriceVal, uint32_t  gasLimitVal,
                                     uint8_t* toStr, uint8_t* valueStr, uint8_t* dataStr, uint8_t* signature) {

    // generate data
    uint8_t encoded[256];
    memset(encoded,0,256);
    uint32_t encoded_len = RlpEncodeForRawTransaction(encoded, nonceVal, gasLimitVal, gasLimitVal, toStr, valueStr, dataStr, signature);
    for (int i = 0; i<encoded_len; i++) {
        printf("%02x", encoded[i]);
    }
}


void Contract::GenerateContractBytes(const char* func, char* out) {
    char intmp[128];
    char outtmp[128];
    memset(intmp, 0, 128);
    memset(outtmp, 0, 128);
    sprintf(intmp, "0x");
    int i = 0;
    for (int i = 0; i<128; i++) {
        char c = func[i];
        if (c == '\0') {
            break;
        }
        sprintf(intmp, "%s%x", intmp, c);
    }

    web3->Web3Sha3(intmp, outtmp);
    strncpy(out, outtmp, 10);
}

void Contract::GenerateBytesForUint(char *output, uint32_t value) {
    // check "M" in uint<M>
    int M = 256;
    char * tmp[4];
    memset(tmp, 0, 4);

    // check number of digits
    char dummy[64];
    int digits = sprintf(dummy, "%x", (uint32_t)value);

    // fill 0 and copy number to string
    sprintf(output, "%s", "0x");
    for(int i = 2; i < 2+64-digits; i++){
        sprintf(output, "%s%s", output, "0");
    }
    sprintf(output,"%s%x", output, (uint32_t)value);
}

void Contract::GenerateBytesForInt(char *output, int32_t value) {
    // check "M" in uint<M>
    int M = 256;
    char * tmp[4];
    memset(tmp, 0, 4);

    // check number of digits
    char dummy[64];
    int digits = sprintf(dummy, "%x", value);

    // fill 0 and copy number to string
    sprintf(output,"%s", "0x");
    char fill[2];
    if (value >= 0) {
        sprintf(fill, "%s", "0");
    } else {
        sprintf(fill, "%s", "f");
    }
    for(int i = 2; i < 2+64-digits; i++){
        sprintf(output, "%s%s", output, fill);
    }
    sprintf(output,"%s%x", output, value);
}

void Contract::GenerateBytesForAddress(char *output, char* value) {
    size_t digits = strlen(value) - 2;

    sprintf(output, "%s", "0x");
    for(int i = 2; i < 2+64-digits; i++){
        sprintf(output, "%s%s", output, "0");
    }
    sprintf(output,"%s%s", output, value+2);

    strlen(value);
}

void Contract::GenerateBytesForString(char *output, char* value) {
    size_t digits = strlen(value);

    sprintf(output, "%s%s", output, "0x");
    for(int i = 0; i < digits; i++){
        sprintf(output, "%s%x", output, value[i]);
    }
    size_t remain = 32 - ((strlen(output)-2) % 32);
    for(int i = 0; i < remain + 32; i++){
        sprintf(output, "%s%s", output, "0");
    }
}

void Contract::GenerateBytesForBytes(char *output, char* value, int len) {
    // check "M" in uint<M>
    int M = 256;
    char * tmp[4];
    memset(tmp, 0, 4);
    sprintf(output, "%s%s", output, "0x");
    for(int i = 0; i < len; i++){
        sprintf(output, "%s%x", output, value[i]);
    }
    size_t remain = 32 - ((strlen(output)-2) % 32);
    for(int i = 0; i < remain + 32; i++){
        sprintf(output, "%s%s", output, "0");
    }

}

uint32_t Contract::RlpEncode(uint8_t* encoded,
                         uint32_t nonceVal, uint32_t gasPriceVal, uint32_t  gasLimitVal,
                         uint8_t* toStr, uint8_t* valueStr, uint8_t* dataStr) {
    uint8_t nonce[4];
    uint32_t lenNonce = Util::ConvertNumberToUintArray(nonce, nonceVal);
    uint8_t gasPrice[4];
    uint32_t lenGasPrice = Util::ConvertNumberToUintArray(gasPrice, gasPriceVal);
    uint8_t gasLimit[4];
    uint32_t lenGasLimit = Util::ConvertNumberToUintArray(gasLimit, gasLimitVal);
    uint8_t to[20];
    uint32_t lenTo = Util::ConvertStringToUintArray(to, toStr);
    uint8_t value[128];
    uint32_t lenValue = Util::ConvertStringToUintArray(value, valueStr);
    uint8_t data[128];
    uint32_t lenData = Util::ConvertStringToUintArray(data, dataStr);

    uint8_t outputNonce[8];
    uint8_t outputGasPrice[8];
    uint8_t outputGasLimit[8];
    uint8_t outputTo[32];
    uint8_t outputValue[8];
    uint8_t outputData[128];
    uint32_t lenOutputNonce;
    uint32_t lenOutputGasPrice;
    uint32_t lenOutputGasLimit;
    uint32_t lenOutputTo;
    uint32_t lenOutputValue;
    uint32_t lenOutputData;

    lenOutputNonce = Util::RlpEncodeItem(outputNonce, nonce, (uint32_t)lenNonce);
    lenOutputGasPrice = Util::RlpEncodeItem(outputGasPrice, gasPrice, (uint32_t)lenGasPrice);
    lenOutputGasLimit = Util::RlpEncodeItem(outputGasLimit, gasLimit, (uint32_t)lenGasLimit);
    lenOutputTo = Util::RlpEncodeItem(outputTo, to, (uint32_t)lenTo);
    lenOutputValue = Util::RlpEncodeItem(outputValue, value, (uint32_t)lenValue);
    lenOutputData = Util::RlpEncodeItem(outputData, data, (uint32_t)lenData);

    uint32_t total_len = Util::RlpEncodeWholeHeader(encoded,
                                           lenOutputNonce+
                                           lenOutputGasPrice+
                                           lenOutputGasLimit+
                                           lenOutputTo+
                                           lenOutputValue+
                                           lenOutputData);
    int p = total_len;
    memcpy(encoded+p, outputNonce, lenOutputNonce);
    p += lenOutputNonce;
    memcpy(encoded+p, outputGasPrice, lenOutputGasPrice);
    p += lenOutputGasPrice;
    memcpy(encoded+p, outputGasLimit, lenOutputGasLimit);
    p += lenOutputGasLimit;
    memcpy(encoded+p, outputTo, lenOutputTo);
    p += lenOutputTo;
    memcpy(encoded+p, outputValue, lenOutputValue);
    p += lenOutputValue;
    memcpy(encoded+p, outputData, lenOutputData);
    p += lenOutputData;

    printf("\nRLP encoded--------\n ");
    printf("\nlength : %d\n", p);
    for (int i = 0; i<p; i++) {
        printf("%02x ", i, encoded[i]);
    }

    return (uint32_t)p;

}


#define SHA256_BLOCK_LENGTH  64
#define SHA256_DIGEST_LENGTH 32

typedef struct SHA256_CTX {
    uint32_t	state[8];
    uint64_t	bitcount;
    uint8_t	buffer[SHA256_BLOCK_LENGTH];
} SHA256_CTX;

typedef struct SHA256_HashContext {
    uECC_HashContext uECC;
    br_sha256_context ctx;
} SHA256_HashContext;

static void init_SHA256(const uECC_HashContext *base) {
    SHA256_HashContext *context = (SHA256_HashContext *)base;
    br_sha256_init(&context->ctx);
}

static void update_SHA256(const uECC_HashContext *base,
                          const uint8_t *message,
                          unsigned message_size) {
    SHA256_HashContext *context = (SHA256_HashContext *)base;
    br_sha256_update(&context->ctx, message, message_size);
}

static void finish_SHA256(const uECC_HashContext *base, uint8_t *hash_result) {
    SHA256_HashContext *context = (SHA256_HashContext *)base;
    br_sha256_out(&context->ctx, hash_result);
}

void Contract::Sign(uint8_t* hash, uint8_t* sig) {
    uint8_t tmp[2 * SHA256_DIGEST_LENGTH + SHA256_BLOCK_LENGTH];
    SHA256_HashContext ctx = {{
                                      &init_SHA256,
                                      &update_SHA256,
                                      &finish_SHA256,
                                      SHA256_BLOCK_LENGTH,
                                      SHA256_DIGEST_LENGTH,
                                      tmp
                              }};

    const struct uECC_Curve_t * curve;
    curve = uECC_secp256k1();

    if (!uECC_sign_deterministic(privateKey, hash, sizeof(hash), &ctx.uECC, sig, curve)) {
        printf("uECC_sign() failed\n");
    }
}

uint32_t Contract::RlpEncodeForRawTransaction(uint8_t* encoded,
                             uint32_t nonceVal, uint32_t gasPriceVal, uint32_t  gasLimitVal,
                             uint8_t* toStr, uint8_t* valueStr, uint8_t* dataStr, uint8_t* sig) {
    uint8_t nonce[4];
    uint32_t lenNonce = Util::ConvertNumberToUintArray(nonce, nonceVal);
    uint8_t gasPrice[4];
    uint32_t lenGasPrice = Util::ConvertNumberToUintArray(gasPrice, gasPriceVal);
    uint8_t gasLimit[4];
    uint32_t lenGasLimit = Util::ConvertNumberToUintArray(gasLimit, gasLimitVal);
    uint8_t to[20];
    uint32_t lenTo = Util::ConvertStringToUintArray(to, toStr);
    uint8_t value[128];
    uint32_t lenValue = Util::ConvertStringToUintArray(value, valueStr);
    uint8_t data[128];
    uint32_t lenData = Util::ConvertStringToUintArray(data, dataStr);

    uint8_t outputNonce[8];
    uint8_t outputGasPrice[8];
    uint8_t outputGasLimit[8];
    uint8_t outputTo[32];
    uint8_t outputValue[8];
    uint8_t outputData[128];
    uint32_t lenOutputNonce;
    uint32_t lenOutputGasPrice;
    uint32_t lenOutputGasLimit;
    uint32_t lenOutputTo;
    uint32_t lenOutputValue;
    uint32_t lenOutputData;

    lenOutputNonce = Util::RlpEncodeItem(outputNonce, nonce, (uint32_t)lenNonce);
    lenOutputGasPrice = Util::RlpEncodeItem(outputGasPrice, gasPrice, (uint32_t)lenGasPrice);
    lenOutputGasLimit = Util::RlpEncodeItem(outputGasLimit, gasLimit, (uint32_t)lenGasLimit);
    lenOutputTo = Util::RlpEncodeItem(outputTo, to, (uint32_t)lenTo);
    lenOutputValue = Util::RlpEncodeItem(outputValue, value, (uint32_t)lenValue);
    lenOutputData = Util::RlpEncodeItem(outputData, data, (uint32_t)lenData);

    uint8_t R[32];
    memcpy(R, sig, 32);
    uint8_t S[32];
    memcpy(S, sig+32, 32);
    uint8_t V[1] = {0x1b};
    uint8_t outputR[33];
    uint8_t outputS[33];
    uint8_t outputV[1];
    uint32_t lenOutputR;
    uint32_t lenOutputS;
    uint32_t lenOutputV;

    lenOutputR = Util::RlpEncodeItem(outputR, R, 32);
    lenOutputS = Util::RlpEncodeItem(outputS, S, 32);
    lenOutputV = Util::RlpEncodeItem(outputV, V, 1);

    printf("\nR--------\n ");
    for (int i = 0; i<lenOutputR; i++) {
        printf("%02x ", outputR[i]);
    }
    printf("\nS--------\n ");
    for (int i = 0; i<lenOutputS; i++) {
        printf("%02x ", outputS[i]);
    }
    printf("\nV--------\n ");
    for (int i = 0; i<lenOutputV; i++) {
        printf("%02x ", outputV[i]);
    }

    uint32_t tx_len = 0;
    tx_len = Util::RlpEncodeWholeHeader(encoded,
                                        lenOutputNonce+
                                        lenOutputGasPrice+
                                        lenOutputGasLimit+
                                        lenOutputTo+
                                        lenOutputValue+
                                        lenOutputData+
                                        lenOutputR+
                                        lenOutputS+
                                        lenOutputV);
    int p = tx_len;
    memcpy(encoded+p, outputNonce, lenOutputNonce);
    p += lenOutputNonce;
    memcpy(encoded+p, outputGasPrice, lenOutputGasPrice);
    p += lenOutputGasPrice;
    memcpy(encoded+p, outputGasLimit, lenOutputGasLimit);
    p += lenOutputGasLimit;
    memcpy(encoded+p, outputTo, lenOutputTo);
    p += lenOutputTo;
    memcpy(encoded+p, outputValue, lenOutputValue);
    p += lenOutputValue;
    memcpy(encoded+p, outputData, lenOutputData);
    p += lenOutputData;
    memcpy(encoded+p, outputV, lenOutputV);
    p += lenOutputV;
    memcpy(encoded+p, outputR, lenOutputR);
    p += lenOutputR;
    memcpy(encoded+p, outputS, lenOutputS);
    p += lenOutputS;
    printf("\nGenerated Transaction--------\n ");
    for (int i = 0; i<p; i++) {
        printf("%02x", encoded[i]);
    }

    return (uint32_t)p;
}
