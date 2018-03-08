//
// Created by Okada, Takahiro on 2018/02/05.
//

#include "Contract.h"
#include "Web3.h"
#include <WiFi.h>
#include "Util.h"
#include "Log.h"

#include "secp256k1/include/secp256k1_recovery.h"
#include "secp256k1/src/secp256k1_c.h"
#include "secp256k1/src/module/recovery/main_impl.h"

static secp256k1_context *ctx;

Contract::Contract(Web3* _web3, const char* address) {
    web3 = _web3;
    contractAddress = address;
    options.gas=0;
    strcpy(options.from,"");
    strcpy(options.to,"");
    strcpy(options.gasPrice,"0");

    ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN);
}

void Contract::SetPrivateKey(uint8_t *key) {
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
    sprintf(out, "%s%s", out, contractBytes);

#if 0
    LOG("==============");
    LOG(contractBytes);
    LOG("==============");
#endif

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

    // 上の個数と型に応じてbyte列を生成
    va_list args;
    va_start(args, paramCount);
    for( int i = 0; i < paramCount; ++i ) {
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

void Contract::SendTransaction(uint8_t *output,
                               uint32_t nonceVal, uint32_t gasPriceVal, uint32_t gasLimitVal,
                               uint8_t *toStr, uint8_t *valueStr, uint8_t *dataStr) {
    uint8_t param[256];
    memset(param,0,256);

    uint8_t signature[64];
    memset(signature,0,64);
    int recid[1] = {1};
    SetupTransactionImpl1(signature, recid, nonceVal, gasPriceVal, gasLimitVal, toStr, valueStr, dataStr);
    uint32_t len = SetupTransactionImpl2(param, nonceVal, gasPriceVal, gasLimitVal, toStr, valueStr, dataStr, signature, recid[0]);

#if 0
    printf("\nGenerated Transaction--------\n ");
    printf("len:%d", (int)len);
    for (int i = 0; i<len; i++) {
        printf("%02x ", param[i]);
    }
#endif
    web3->EthSendSignedTransaction(param, len, (char *)output);

}

void Contract::SetupTransactionImpl1(uint8_t* signature, int* recid, uint32_t nonceVal, uint32_t gasPriceVal, uint32_t  gasLimitVal,
                                uint8_t* toStr, uint8_t* valueStr, uint8_t* dataStr) {
    // encode
    uint8_t encoded[256];
    memset(encoded,0,256);
    char tmp[256];
    memset(tmp,0,256);
    uint32_t encoded_len = RlpEncode(encoded, nonceVal, gasPriceVal, gasLimitVal, toStr, valueStr, dataStr);

    // hash
    string t = string(tmp);
    Util::BufToString(tmp, encoded, encoded_len);
    string hashedStr = web3->Web3Sha3(&t);

    // sign
    memset(tmp,0,256);
    Util::ConvertStringToUintArray((uint8_t*)tmp, (uint8_t*)hashedStr.c_str());
    Sign((uint8_t*)tmp, signature, recid);

#if 0
    printf("\nhash_input : %s\n", tmp);
    printf("\nhash_output: %s\n", hashedStr);
    printf("\nhash--------\n ");
    for (int i = 0; i<32; i++) {
        printf("%02x ", tmp[i]);
    }
#endif
}

uint32_t Contract::SetupTransactionImpl2(uint8_t* out,
                                     uint32_t nonceVal, uint32_t gasPriceVal, uint32_t  gasLimitVal,
                                     uint8_t* toStr, uint8_t* valueStr, uint8_t* dataStr, uint8_t* signature, uint8_t recid) {

    // generate data
    uint32_t encoded_len = RlpEncodeForRawTransaction(out, nonceVal, gasPriceVal, gasLimitVal, toStr, valueStr, dataStr, signature, recid);
    return encoded_len;
}


void Contract::GenerateContractBytes(const char* func, char* out) {
    char intmp[128];
    memset(intmp, 0, 128);
    sprintf(intmp, "0x");
    int i = 0;
    for (int i = 0; i<128; i++) {
        char c = func[i];
        if (c == '\0') {
            break;
        }
        sprintf(intmp, "%s%x", intmp, c);
    }

    string in = string(intmp);
    string outtmp = web3->Web3Sha3(&in);
    strncpy(out, outtmp.c_str(), 10);
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
//    sprintf(output, "%s", "0x");
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
    // sprintf(output,"%s", "0x");
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

    // sprintf(output, "%s", "0x");
    for(int i = 2; i < 2+64-digits; i++){
        sprintf(output, "%s%s", output, "0");
    }
    sprintf(output,"%s%s", output, value+2);

    strlen(value);
}

void Contract::GenerateBytesForString(char *output, char* value) {
    size_t digits = strlen(value);

    // sprintf(output, "%s%s", output, "0x");
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
    // sprintf(output, "%s%s", output, "0x");
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

#if 0
    printf("\nRLP encoded--------\n ");
    printf("\nlength : %d\n", p);
    for (int i = 0; i<p; i++) {
        printf("%02x ", i, encoded[i]);
    }
#endif

    return (uint32_t)p;
}

void Contract::Sign(uint8_t* hash, uint8_t* sig, int* recid) {
    secp256k1_nonce_function noncefn = secp256k1_nonce_function_rfc6979;
    void* data_ = NULL;

    secp256k1_ecdsa_recoverable_signature signature;
    memset(&signature, 0, sizeof(signature));
    if (secp256k1_ecdsa_sign_recoverable(ctx, &signature, hash,  privateKey, noncefn, data_) == 0) {
        return;
    }

    secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, &sig[0], &recid[0], &signature);

#if 0
    printf("\nhash--------\n ");
    for (int i = 0; i<32; i++) {
        printf("%02x ", hash[i]);
    }
    printf("\npriv--------\n ");
    for (int i = 0; i<32; i++) {
        printf("%02x ", privateKey[i]);
    }
    printf("\nsignature--------\n ");
    for (int i = 0; i<64; i++) {
        printf("%02x ", sig[i]);
    }
#endif
}

uint32_t Contract::RlpEncodeForRawTransaction(uint8_t* encoded,
                             uint32_t nonceVal, uint32_t gasPriceVal, uint32_t  gasLimitVal,
                             uint8_t* toStr, uint8_t* valueStr, uint8_t* dataStr, uint8_t* sig, uint8_t recid) {
    uint8_t nonce[4];
    uint8_t gasPrice[4];
    uint8_t gasLimit[4];
    uint8_t to[20];
    uint8_t value[128];
    uint8_t data[128];

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

    uint32_t lenNonce = Util::ConvertNumberToUintArray(nonce, nonceVal);
    uint32_t lenGasPrice = Util::ConvertNumberToUintArray(gasPrice, gasPriceVal);
    uint32_t lenGasLimit = Util::ConvertNumberToUintArray(gasLimit, gasLimitVal);
    uint32_t lenTo = Util::ConvertStringToUintArray(to, toStr);
    uint32_t lenValue = Util::ConvertStringToUintArray(value, valueStr);
    uint32_t lenData = Util::ConvertStringToUintArray(data, dataStr);

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
    uint8_t V[1] = {(uint8_t)(recid+27)};
    uint8_t outputR[33];
    uint8_t outputS[33];
    uint8_t outputV[1];
    uint32_t lenOutputR;
    uint32_t lenOutputS;
    uint32_t lenOutputV;

    lenOutputR = Util::RlpEncodeItem(outputR, R, 32);
    lenOutputS = Util::RlpEncodeItem(outputS, S, 32);
    lenOutputV = Util::RlpEncodeItem(outputV, V, 1);

#if 0
    printf("\noutputNonce--------\n ");
    for (int i = 0; i<lenOutputNonce; i++) {
        printf("%02x ", outputNonce[i]);
    }
    printf("\noutputGasPrice--------\n ");
    for (int i = 0; i<lenOutputGasPrice; i++) {
        printf("%02x ", outputGasPrice[i]);
    }
    printf("\noutputGasLimit--------\n ");
    for (int i = 0; i<lenOutputGasLimit; i++) {
        printf("%02x ", outputGasLimit[i]);
    }
    printf("\noutputTo--------\n ");
    for (int i = 0; i<lenOutputTo; i++) {
        printf("%02x ", outputTo[i]);
    }
    printf("\noutputValue--------\n ");
    for (int i = 0; i<lenOutputValue; i++) {
        printf("%02x ", outputValue[i]);
    }
    printf("\noutputData--------\n ");
    for (int i = 0; i<lenOutputData; i++) {
        printf("%02x ", outputData[i]);
    }
    printf("\nR--------\n ");
    for (int i = 0; i<lenOutputR; i++) {
        printf("%02x ", outputR[i]);
    }
    printf("\nS--------\n ");
    for (int i = 0; i<lenOutputS; i++) {
        printf("%02x ", outputS[i]);
    }
    printf("\nV[%d]--------\n ", lenOutputV);
    for (int i = 0; i<lenOutputV; i++) {
        printf("%02x ", outputV[i]);
    }
    printf("\n");
#endif

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

    return (uint32_t)p;
}
