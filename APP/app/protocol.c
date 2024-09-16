#include "protocol.h"
#include "cJSON.h"
#include "driver_tool.h"

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
 * }*/



//char receiveData[] = {'{"hd":"H3.1S1.0","v":{"U":1,"D":1,"L":1,"R":1,"A":1,"B":1,"X":1,"Y":1,"LB":1,"RB":1,"ST":1,"SE":1,"RK":1,"LK":1,"LX":1,"LY":1,"RX":1,"RY":1,},"D":1}'};

//char receiveData[] = "{\"hd\":\"H3.1S1.0\",\"v\":{\"U\":1,\"D\":1,\"L\":1,\"R\":1}}";

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

char pro_value_name[][32] = {
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

#define GAMEPAD_PROTOCOL_V1 "H3.1S1.0"

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
            GUA_LOGI("p_value_data: %d", p_value_data->valueint);
            return p_value_data->valueint;
        }
    }
    return REV_OK;
}

static int32 protocol_decode_value(cJSON *root, KEY_DETECTION *key_value)
{
    int32 ret = 0;
    uint32 i = 0;
    for (i=PRO_VALUE_LEFT_E; i < (sizeof(pro_value_name) / sizeof(pro_value_name[0])); i++) {
    	ret = protocol_decode_value_int_data(root, pro_value_name[i]);
		if (ret == REV_ERR) {
			return REV_ERR;
		}
		switch(i){
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
    GUA_LOGI("get json data:%s", json_str);

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
