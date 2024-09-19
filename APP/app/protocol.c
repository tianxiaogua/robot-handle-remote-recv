/*
 * 遥控手柄的网络部分的无线数据协议通过json形式解析
 *
 * {
 * 	"hd":"H3.1S1.0",
 * 	"v":
 * 		{
 * 			"U":1,
 * 			"D":1,
 * 			"L":1,
 * 			"R":1,
 * 			"A":1,
 * 			"B":1,
 * 			"X":1,
 * 			"Y":1,
 * 			"LB":1,
 * 			"RB":1,
 * 			"ST":1,
 * 			"SE":1,
 * 			"R":1,
 * 			"L":1,
 * 			"LX":1,
 * 			"LY":1,
 * 			"RX":1,
 * 			"RY":1,
 * 		},
 *  "D":1
 * }
 * 
 * test data: {"hd":"H3.1S1.0","v":{"U":1,"D":2,"L":3,"R":4,"A":5,"B":6,"X":7,"Y":8,"LB":9,"RB":10,"ST":11,"SE":12,"RK":13,"LK":14,"LX":15,"LY":16,"RX":17,"RY":18}}
 * */

#include "protocol.h"
#include "cJSON.h"
#include "driver_tool.h"

#define GAMEPAD_PROTOCOL_V1 "H3.1S1.0"

#define PRO_HEAD          "hd"
#define PRO_VALUE         "v"
#define PRO_VALUE_LEFT    "L"
#define PRO_VALUE_RIGHT   "R"
#define PRO_VALUE_UP      "U"
#define PRO_VALUE_DOWN    "D"
#define PRO_VALUE_A       "A"
#define PRO_VALUE_B       "B"
#define PRO_VALUE_X       "X"
#define PRO_VALUE_Y       "Y"
#define PRO_VALUE_LB      "LB"
#define PRO_VALUE_RB      "RB"
#define PRO_VALUE_SELECT  "SE"
#define PRO_VALUE_START   "ST"
#define PRO_VALUE_R_KEY   "RK"
#define PRO_VALUE_L_KEY   "LK"
#define PRO_VALUE_LX      "LX"
#define PRO_VALUE_LY      "LY"
#define PRO_VALUE_RX      "RX"
#define PRO_VALUE_RY      "RY"

enum PRO_NAME
{
	PRO_HEAD_E = 0,
	PRO_VALUE_E,
	PRO_VALUE_LEFT_E,
	PRO_VALUE_RIGHT_E,
	PRO_VALUE_UP_E,
	PRO_VALUE_DOWN_E,
	PRO_VALUE_A_E ,
	PRO_VALUE_B_E,
	PRO_VALUE_X_E,
	PRO_VALUE_Y_E,
	PRO_VALUE_LB_E,
	PRO_VALUE_RB_E,
	PRO_VALUE_SELECT_E,
	PRO_VALUE_START_E,
	PRO_VALUE_R_KEY_E,
	PRO_VALUE_L_KEY_E,
	PRO_VALUE_LX_E,
	PRO_VALUE_LY_E,
	PRO_VALUE_RX_E,
	PRO_VALUE_RY_E,
};

static char pro_value_name[][32] = {
		PRO_HEAD,
		PRO_VALUE,
		PRO_VALUE_LEFT,
		PRO_VALUE_RIGHT,
		PRO_VALUE_UP,
		PRO_VALUE_DOWN,
		PRO_VALUE_A ,
		PRO_VALUE_B,
		PRO_VALUE_X,
		PRO_VALUE_Y,
		PRO_VALUE_LB,
		PRO_VALUE_RB,
		PRO_VALUE_SELECT,
		PRO_VALUE_START,
		PRO_VALUE_R_KEY,
		PRO_VALUE_L_KEY,
		PRO_VALUE_LX,
		PRO_VALUE_LY,
		PRO_VALUE_RX,
		PRO_VALUE_RY,
};

/**
 * @brief 把value字段的具体键值解析出数据
 * 
 * @param p_value json字段
 * @param name  要解析的字段的名字
 * @return int32 
 */
static int32 protocol_decode_value_int_data(cJSON *p_value, char *name)
{
    cJSON *p_value_data = NULL;

    if (p_value == NULL) {
        GUA_LOGE("input json object is null!");
        return REV_ERR;
    }

    p_value_data = cJSON_GetObjectItem(p_value, name);        
    if (!p_value_data) {
        GUA_LOGE("json data is error! get name:%s", name);
        return REV_ERR;
    } else {
        if (cJSON_IsNumber(p_value_data)) {
//            GUA_LOGI("p_value_data: %d", p_value_data->valueint);
            return p_value_data->valueint;
        }
    }
    return REV_OK;
}

/**
 * @brief 解析value字段
 * 
 * @param root 输入的json字段
 * @param key_value 输出的数据
 * @return int32 
 */
static int32 protocol_decode_value(cJSON *root, KEY_DETECTION *key_value)
{
    int32 ret = 0;
    uint32 i = 0;
    for (i=PRO_VALUE_LEFT_E; i < (sizeof(pro_value_name) / sizeof(pro_value_name[0])); i++) {
    	ret = protocol_decode_value_int_data(root, pro_value_name[i]);
		if (ret == REV_ERR) {
			return REV_ERR;
		}
		switch(i) {
		case PRO_VALUE_LEFT_E:   key_value->KEY_LEFT = ret; break;
		case PRO_VALUE_RIGHT_E:  key_value->KEY_RIGHT = ret; break;
		case PRO_VALUE_UP_E:     key_value->KEY_UP_ = ret; break;
		case PRO_VALUE_DOWN_E:   key_value->KEY_DOWN_ = ret; break;
		case PRO_VALUE_A_E:      key_value->KEY_A = ret; break;
		case PRO_VALUE_B_E:      key_value->KEY_B = ret; break;
		case PRO_VALUE_X_E:      key_value->KEY_X = ret; break;
		case PRO_VALUE_Y_E:      key_value->KEY_Y = ret; break;
		case PRO_VALUE_LB_E:     key_value->KEY_LB = ret; break;
		case PRO_VALUE_RB_E:     key_value->KEY_RB = ret; break;
		case PRO_VALUE_SELECT_E: key_value->KEY_SELECT = ret; break;
		case PRO_VALUE_START_E:  key_value->KEY_START = ret; break;
		case PRO_VALUE_R_KEY_E:  key_value->KEY_R_KEY = ret; break;
		case PRO_VALUE_L_KEY_E:  key_value->KEY_L_KEY = ret; break;
		case PRO_VALUE_LX_E:     key_value->LX = ret; break;
		case PRO_VALUE_LY_E:     key_value->LY = ret; break;
		case PRO_VALUE_RX_E:     key_value->RX = ret; break;
		case PRO_VALUE_RY_E:     key_value->RY = ret; break;
		}
    }
    return REV_OK;
}

/**
 * @brief 解析json协议
 * 
 * @param json_str 输入的字符串
 * @param key_value 输出的解析好的数据
 * @return int32 
 */
int32 protocol_decode(const char *json_str, KEY_DETECTION *key_value)
{
    cJSON *p_json_root; // 根节点
    cJSON *p_head;
    cJSON *p_value;

    p_json_root = cJSON_Parse(json_str); //首先整体判断是否为一个json格式的数据
    //如果是否json格式数据
    if (!p_json_root) {
        GUA_LOGE("input str is not json!");
    }
//    GUA_LOGI("get json data:%s", json_str);

    char protocol_head[32] = {0};
    p_head = cJSON_GetObjectItem(p_json_root, PRO_HEAD);    // 解析mac字段字符串内容
    if (!p_head) {
        GUA_LOGE("json data is error! json data:%s", json_str);
        return REV_ERR;
    } else {
        if (cJSON_IsString(p_head)) {
            strcpy(protocol_head, p_head->valuestring);               // 拷贝内容到字符串数组
            GUA_LOGI("protocol_head: %s", protocol_head);
        }
    }

    if (strcmp(GAMEPAD_PROTOCOL_V1, protocol_head) == 0) {
        GUA_LOGI("get gamepad protocol");

        p_value = cJSON_GetObjectItem(p_json_root, PRO_VALUE);      // 解析value字段内容
        if (!p_value) {
            GUA_LOGE("json data is error! json data:%s", json_str);
            return REV_ERR;
        } else {
            protocol_decode_value(p_value, key_value);
        }
    }
    
    return REV_OK;
}

/**
 * @brief 编码json数据
 * 
 * @param json_str 输出的编码字符串
 * @param key_value 输入的要编码的数据
 * @return int32 
 */
int32 protocol_encode(char *json_str, KEY_DETECTION *key_value)
{
	cJSON *p_root;
	cJSON *p_value;
	cJSON *ret;
	int32 value_data = 0;
	uint32 i = 0;
	char *json_buf;

	p_root = cJSON_CreateObject(); // 创建JSON根部结构体
	if (!p_root) {
		GUA_LOGE("cjson create error!");
	}
	p_value = cJSON_CreateObject(); // 创建JSON子叶结构体
	if (!p_value) {
		GUA_LOGE("cjson create error!");
	}

	cJSON_AddStringToObject(p_root, PRO_HEAD, GAMEPAD_PROTOCOL_V1); // 添加字符串类型数据到根部结构体
	cJSON_AddItemToObject(p_root, PRO_VALUE, p_value);

    for (i=PRO_VALUE_LEFT_E; i < (sizeof(pro_value_name) / sizeof(pro_value_name[0])); i++) {
		switch(i) {
		case PRO_VALUE_LEFT_E:   value_data = key_value->KEY_LEFT; break;
		case PRO_VALUE_RIGHT_E:  value_data = key_value->KEY_RIGHT; break;
		case PRO_VALUE_UP_E:     value_data = key_value->KEY_UP_; break;
		case PRO_VALUE_DOWN_E:   value_data = key_value->KEY_DOWN_; break;
		case PRO_VALUE_A_E:      value_data = key_value->KEY_A; break;
		case PRO_VALUE_B_E:      value_data = key_value->KEY_B; break;
		case PRO_VALUE_X_E:      value_data = key_value->KEY_X; break;
		case PRO_VALUE_Y_E:      value_data = key_value->KEY_Y; break;
		case PRO_VALUE_LB_E:     value_data = key_value->KEY_LB; break;
		case PRO_VALUE_RB_E:     value_data = key_value->KEY_RB; break;
		case PRO_VALUE_SELECT_E: value_data = key_value->KEY_SELECT; break;
		case PRO_VALUE_START_E:  value_data = key_value->KEY_START; break;
		case PRO_VALUE_R_KEY_E:  value_data = key_value->KEY_R_KEY; break;
		case PRO_VALUE_L_KEY_E:  value_data = key_value->KEY_L_KEY; break;
		case PRO_VALUE_LX_E:     value_data = key_value->LX; break;
		case PRO_VALUE_LY_E:     value_data = key_value->LY; break;
		case PRO_VALUE_RX_E:     value_data = key_value->RX; break;
		case PRO_VALUE_RY_E:     value_data = key_value->RY; break;
		default : GUA_LOGE("input json value error!");
		}
		ret = cJSON_AddNumberToObject(p_value, pro_value_name[i], value_data); 
		if (ret == NULL) {
			GUA_LOGE("cjson add error!");
		}
    }
	json_buf = cJSON_PrintUnformatted(p_root);
	strcpy(json_str, json_buf);
	GUA_LOGI("out json:%s", json_buf);
	return REV_OK;
}
