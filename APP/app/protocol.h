#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "driver_tool.h"
#include "keyscan.h"

/**
 * @brief 解析json协议
 * 
 * @param json_str 输入的字符串
 * @param key_value 输出的解析好的数据
 * @return int32 
 */
int32 protocol_decode(const char *json_str, KEY_DETECTION *key_value);

/**
 * @brief 编码json数据
 * 
 * @param json_str 输出的编码字符串
 * @param key_value 输入的要编码的数据
 * @return int32 
 */
int32 protocol_encode(char *json_str, KEY_DETECTION *key_value);
#endif


