//
// Created by Okada, Takahiro on 2018/02/11.
//

#ifndef WEB3_UTIL_H
#define WEB3_UTIL_H

#include <stdint.h>

class Util {
public:
    // RLP implementation
    // reference:
    //     https://github.com/ethereum/wiki/wiki/%5BEnglish%5D-RLP
    static uint32_t RlpEncodeWholeHeader(uint8_t *header_output, uint32_t total_len);
    static uint32_t RlpEncodeItem(uint8_t* output, const uint8_t* input, uint32_t input_len);

    static uint32_t ConvertNumberToUintArray(uint8_t *str, uint32_t val);
    static uint32_t ConvertStringToUintArray(uint8_t *out, uint8_t *in);
    static uint8_t HexToInt(uint8_t s);
    static void BufToString(char* str, const uint8_t* buf, uint32_t len);

};

#endif //WEB3_UTIL_H
