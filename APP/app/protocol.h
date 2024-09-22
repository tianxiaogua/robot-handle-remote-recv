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
int32 protocol_decode_json(const char *json_str, KEY_DETECTION *key_value);

/**
 * @brief 编码json数据
 * 
 * @param json_str 输出的编码字符串
 * @param key_value 输入的要编码的数据
 * @return int32 
 */
int32 protocol_encode_json(char *json_str, KEY_DETECTION *key_value);


/**
 * @brief 将输入数据编码成为protoc数据格式
 * 
 * @param key_value 输入的键值
 * @param buf 输出的数据
 * @param buf_len 输出的数据长度
 * @return int32 
 */
int32 app_protocol_encode_protoc(KEY_DETECTION *key_value, uint8 *buf, uint32 buf_len);


/**
 * @brief 将输入的编码数据解码成为数据输出
 * 
 * @param buf 输出数据
 * @param buf_len 输出数据长度
 * @param key_value 输入键值
 * @return int32 
 */
int32 app_protocol_decode_protoc(uint8 *buf, uint32 buf_len, KEY_DETECTION *key_value);


#endif


